/*
 * Copyright (C) 2013 INRIA
 *               2015 Kaspar Schleiser <kaspar@schleiser.de>
 *               2016 Eistec AB
 *               2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     sys
 * @{
 *
 * @file
 * @brief       STDIO over UART implementation
 *
 * This file implements a UART callback and the STDIO read/write functions
 *
 * @author      Oliver Hahm <oliver.hahm@inria.fr>
 * @author      Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Joakim Nohlgård <joakim.nohlgard@eistec.se>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <errno.h>
#include <string.h>

#include "stdio_base.h"
#include "stdio_uart_config.h"
#include "stm32f4xx_hal.h"
#include "hal_errno.h"

#include "dma.h"
#include "gpio.h"
#include "rcc.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

typedef struct
{
    uint8_t *pbuf;
    uint16_t size;
} uart_tx_data_t;

static SemaphoreHandle_t _tx_cplt_semphr = NULL;
static StaticSemaphore_t _tx_cplt_semphr_storage;

static StaticQueue_t _rx_queue_struct;
static uint8_t _rx_queue_storage[STDIO_UART_RX_QUEUE_LENGTH * sizeof(uint8_t)];
static QueueHandle_t _rx_queue = NULL;

static StaticQueue_t _tx_avail_queue_struct;
static uint8_t _tx_avail_queue_storage[STDIO_UART_TX_AVAIL_QUEUE_LENGTH * sizeof(uint8_t *)];
static QueueHandle_t _tx_avail_queue = NULL;

static StaticQueue_t _tx_ready_queue_struct;
static uint8_t _tx_ready_queue_storage[STDIO_UART_TX_READY_QUEUE_LENGTH * sizeof(uart_tx_data_t)];
static QueueHandle_t _tx_ready_queue = NULL;

static StackType_t _write_task_stack[STDIO_UART_WRITE_TASK_STACKSIZE];
static StaticTask_t _write_task_tcb;
static TaskHandle_t h_write_task = NULL;

static StackType_t _read_task_stack[STDIO_UART_READ_TASK_STACKSIZE];
static StaticTask_t _read_task_tcb;
static TaskHandle_t h_read_task = NULL;

static SemaphoreHandle_t _stdin_mutex = NULL;
static StaticSemaphore_t _stdin_mutex_storage;

static StaticQueue_t _stdin_queue_struct;
static uint8_t _stdin_queue_storage[STDIO_UART_STDIN_QUEUE_LENGTH * sizeof(uint8_t)];
static QueueHandle_t _stdin_queue = NULL;

UART_HandleTypeDef h_stdio_uart;

static uint8_t _tx_buffer[STDIO_UART_TX_BUFFER_DEPTH * STDIO_UART_TX_AVAIL_QUEUE_LENGTH];
static uint8_t _rx_buffer;

static const TickType_t dma_tx_max_time_ms = (TickType_t)((1000.0f * (((float)(10 * STDIO_UART_TX_BUFFER_DEPTH)) / 115200.0f)) + 0.5f);

static HAL_StatusTypeDef _error = HAL_OK;

static QueueHandle_t _stdin_listeners_list[STDIO_UART_MAX_NUM_OF_STDIN_LISTENERS];
static uint32_t _stdin_listeners = 0ul;

static void _uart_msp_init(UART_HandleTypeDef *huart);
static void _uart_msp_deinit(UART_HandleTypeDef *huart);
static void _tx_cplt_callback(UART_HandleTypeDef *huart);
static void _rx_cplt_callback(UART_HandleTypeDef *huart);
static void _uart_error_callback(UART_HandleTypeDef *huart);

static void _write_task(void *params);
static void _read_task(void *params);
static inline void stdin_lock(void);
static inline void stdin_unlock(void);
static int _uart_write(const uint8_t *data, size_t len);
static void _error_handler(void);

int stdio_uart_add_stdin_listener(const QueueHandle_t hqueue)
{
    assert(hqueue);

    int ret;

    if (_stdin_listeners < STDIO_UART_MAX_NUM_OF_STDIN_LISTENERS)
    {
        _stdin_listeners_list[_stdin_listeners] = hqueue;
        _stdin_listeners++;
        ret = 0;
    }
    else
    {
        ret = -ENOMEM;
    }

    return ret;
}

/**
 * @brief  UART writer gatekeeper task
 * @param  params not used
 * @retval None
 * @note   Task that performs the UART TX related jobs.
 */
static void _write_task(void *params)
{
    (void)params;

    BaseType_t ret;
    HAL_StatusTypeDef hal_status;
    uint8_t *_tx_pending = NULL;

    uart_tx_data_t uart_tx_data = {
        .pbuf = NULL,
        .size = 0,
    };

    const TickType_t ticks_to_wait = pdMS_TO_TICKS(2ul * dma_tx_max_time_ms);

    for ( ;; )
    {
        ret = xQueueReceive(_tx_ready_queue, &uart_tx_data, portMAX_DELAY);
        assert(ret);

        hal_status = HAL_UART_Transmit_DMA(&h_stdio_uart, uart_tx_data.pbuf, uart_tx_data.size);
        if (HAL_OK != hal_status)
        {
            _error_handler();
        }
        assert(HAL_OK == hal_status);

        _tx_pending = uart_tx_data.pbuf;

        ret = xSemaphoreTake(_tx_cplt_semphr, ticks_to_wait);
        assert(ret);
        ret = xQueueSend(_tx_avail_queue, &_tx_pending, 0);
        assert(ret);
    }
}

static void _read_task(void *params)
{
    (void)params;

    HAL_StatusTypeDef hal_status = HAL_UART_Receive_IT(&h_stdio_uart, &_rx_buffer, 1);
    assert(HAL_OK == hal_status);

    for ( ;; )
    {
        uint8_t rx_data;
        BaseType_t ret = xQueueReceive(_rx_queue, &rx_data, portMAX_DELAY);
        assert(ret);

        hal_status = HAL_UART_Receive_IT(&h_stdio_uart, &_rx_buffer, 1);
        assert(HAL_OK == hal_status);

        if (NULL != xSemaphoreGetMutexHolder(_stdin_mutex))
        {
            ret = xQueueSend(_stdin_queue, &rx_data, portMAX_DELAY);
            assert(ret);
        }
        else
        {
            for (uint32_t i = 0; i < _stdin_listeners; i++)
            {
                ret = xQueueSend(_stdin_listeners_list[i], &rx_data, portMAX_DELAY);
                assert(ret);
            }
        }
    }
}

/**
 * @brief  Initializes the STDIO_UART peripheral
 *
 * @param  None
 *
 * @return  0 for success
 * @return < 0 an error occurred
 *
 * @note   The communication is configured 115200 Baudrate 8N1 with no
 *         flowcontrol.
 */
int stdio_uart_init(void)
{
    _stdin_listeners = 0ul;

    _tx_cplt_semphr = xSemaphoreCreateBinaryStatic(&_tx_cplt_semphr_storage);
    assert(_tx_cplt_semphr);

    _tx_avail_queue = xQueueCreateStatic(STDIO_UART_TX_AVAIL_QUEUE_LENGTH,
                                         sizeof(uint8_t *),
                                         _tx_avail_queue_storage,
                                         &_tx_avail_queue_struct);
    assert(_tx_avail_queue);

    _tx_ready_queue = xQueueCreateStatic(STDIO_UART_TX_READY_QUEUE_LENGTH,
                                         sizeof(uart_tx_data_t),
                                         _tx_ready_queue_storage,
                                         &_tx_ready_queue_struct);
    assert(_tx_ready_queue);

    _rx_queue = xQueueCreateStatic(STDIO_UART_RX_QUEUE_LENGTH,
                                   sizeof(uint8_t),
                                   _rx_queue_storage,
                                   &_rx_queue_struct);
    assert(_rx_queue);

    _stdin_mutex = xSemaphoreCreateMutexStatic(&_stdin_mutex_storage);
    assert(_stdin_mutex);

    _stdin_queue = xQueueCreateStatic(STDIO_UART_STDIN_QUEUE_LENGTH,
                                      sizeof(uint8_t),
                                      _stdin_queue_storage,
                                      &_stdin_queue_struct);
    assert(_stdin_queue);

    for (uint8_t i = 0; i < STDIO_UART_TX_AVAIL_QUEUE_LENGTH; i++)
    {
        uint8_t *buffer_address = &_tx_buffer[i * STDIO_UART_TX_BUFFER_DEPTH];
        BaseType_t retv = xQueueSend(_tx_avail_queue, &buffer_address, 0);
        assert(retv);
    }

    h_write_task = xTaskCreateStatic(_write_task,
                                     "STDIO UART Write",
                                     STDIO_UART_WRITE_TASK_STACKSIZE,
                                     NULL,
                                     STDIO_UART_WRITE_TASK_PRIORITY,
                                     _write_task_stack,
                                     &_write_task_tcb);
    assert(h_write_task);

    h_read_task = xTaskCreateStatic(_read_task,
                                    "STDIO UART Read",
                                    STDIO_UART_READ_TASK_STACKSIZE,
                                    NULL,
                                    STDIO_UART_READ_TASK_PRIORITY,
                                    _read_task_stack,
                                    &_read_task_tcb);
    assert(h_read_task);

    _error = HAL_OK;

    h_stdio_uart.Instance = STDIO_UART_USARTx;
    h_stdio_uart.Init.BaudRate = 115200ul;
    h_stdio_uart.Init.WordLength = UART_WORDLENGTH_8B;
    h_stdio_uart.Init.StopBits = UART_STOPBITS_1;
    h_stdio_uart.Init.Parity = UART_PARITY_NONE;
    h_stdio_uart.Init.Mode = UART_MODE_TX_RX;
    h_stdio_uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    h_stdio_uart.Init.OverSampling = UART_OVERSAMPLING_16;

    HAL_StatusTypeDef ret;

    ret = HAL_UART_RegisterCallback(&h_stdio_uart, HAL_UART_MSPINIT_CB_ID, _uart_msp_init);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_UART_RegisterCallback(&h_stdio_uart, HAL_UART_MSPDEINIT_CB_ID, _uart_msp_deinit);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_UART_Init(&h_stdio_uart);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    if (HAL_OK != _error)
    {
        return hal_statustypedef_to_errno(_error);
    }

    ret = HAL_UART_RegisterCallback(&h_stdio_uart, HAL_UART_TX_COMPLETE_CB_ID, _tx_cplt_callback);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_UART_RegisterCallback(&h_stdio_uart, HAL_UART_RX_COMPLETE_CB_ID, _rx_cplt_callback);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_UART_RegisterCallback(&h_stdio_uart, HAL_UART_ERROR_CB_ID, _uart_error_callback);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    return 0;
}


/**
 * @brief  De-initializes the STDIO_UART peripheral
 *
 * @param  None
 *
 * @return  0 for success
 * @return < 0 an error occurred
 * @note   -
 */
int stdio_uart_deinit(void)
{
    HAL_StatusTypeDef ret;

    ret = HAL_UART_AbortReceive(&h_stdio_uart);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_UART_AbortTransmit(&h_stdio_uart);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_UART_UnRegisterCallback(&h_stdio_uart, HAL_UART_TX_COMPLETE_CB_ID);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_UART_UnRegisterCallback(&h_stdio_uart, HAL_UART_RX_COMPLETE_CB_ID);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_UART_UnRegisterCallback(&h_stdio_uart, HAL_UART_ERROR_CB_ID);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_UART_DeInit(&h_stdio_uart);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    if (HAL_OK != _error)
    {
        return hal_statustypedef_to_errno(_error);
    }

    ret = HAL_UART_UnRegisterCallback(&h_stdio_uart, HAL_UART_MSPINIT_CB_ID);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_UART_UnRegisterCallback(&h_stdio_uart, HAL_UART_MSPDEINIT_CB_ID);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    vTaskDelete(h_write_task);
    vTaskDelete(h_read_task);
    vQueueDelete(_tx_avail_queue);
    vQueueDelete(_tx_ready_queue);
    vQueueDelete(_rx_queue);
    vQueueDelete(_stdin_queue);
    vSemaphoreDelete(_tx_cplt_semphr);
    vSemaphoreDelete(_stdin_mutex);

    h_write_task = NULL;
    h_read_task = NULL;
    _tx_avail_queue = NULL;
    _tx_ready_queue = NULL;
    _rx_queue = NULL;
    _stdin_queue = NULL;
    _tx_cplt_semphr = NULL;
    _stdin_mutex = NULL;

    return 0;
}

ssize_t stdio_uart_read(void *buffer, size_t count)
{
    assert(buffer);
    uint8_t *buf = buffer;

    stdin_lock();

    for (size_t i = 0; i < count; i++)
    {
        BaseType_t ret = xQueueReceive(_stdin_queue, &buf[i], portMAX_DELAY);
        assert(ret);
    }

    stdin_unlock();

    return count;
}

ssize_t stdio_uart_write(const void *buffer, size_t len)
{
    ssize_t result = len;

    if (IS_USED(MODULE_STDIO_UART_ONLCR))
    {
        static const uint8_t crlf[2] = { (uint8_t)'\r', (uint8_t)'\n' };
        const uint8_t *buf = buffer;

        while (len)
        {
            const uint8_t *pos = memchr(buf, '\n', len);
            size_t chunk_len = (pos != NULL && len != 1)
                             ? (uintptr_t)pos - (uintptr_t)buf
                             : len;
            int ret = _uart_write(buf, chunk_len);
            if (ret < 0)
            {
                return ret;
            }

            buf += chunk_len;
            len -= chunk_len;

            if (len)
            {
                ret = _uart_write(crlf, sizeof(crlf));
                if (ret < 0)
                {
                    return ret;
                }

                buf++;
                len--;
            }
        }
    }
    else
    {
        int ret = _uart_write((const uint8_t *)buffer, len);
        if (ret < 0)
        {
            return ret;
        }
    }
    return result;
}

ssize_t stdio_uart_write_blocking(const void *buffer, size_t len)
{
    HAL_StatusTypeDef ret;
    ret = HAL_UART_DMAPause(&h_stdio_uart);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_UART_Transmit(&h_stdio_uart, (uint8_t *)buffer, (uint16_t)len, 0xFFFFFFFFul);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_UART_DMAResume(&h_stdio_uart);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    return (ssize_t)len;
}

/**
 * @brief  Initializes the STDIO_UART MSP (low-level)
 * @param  huart pointer to the UART_HandleTypeDef structure
 * @retval None
 * @note   This function initializes the GPIOs corresponding the UART peripheral,
 *         the DMA stream and enables the DMA and UART interrupts
 */
static void _uart_msp_init(UART_HandleTypeDef *huart)
{
    rcc_periph_clk_enable((const void *)STDIO_UART_USARTx);
    rcc_periph_reset((const void *)STDIO_UART_USARTx);

    stdio_uart_tx_pin_init();
    stdio_uart_rx_pin_init();

    _error = stdio_uart_dma_init(huart);

    HAL_NVIC_SetPriority(STDIO_UART_USARTx_IRQn, STDIO_UART_USARTx_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(STDIO_UART_USARTx_IRQn);
}

/**
 * @brief  De-initializes the STDIO_UART MSP (low-level)
 * @param  huart pointer to the UART_HandleTypeDef structure
 * @retval None
 * @note   This function de-initializes the GPIOs corresponding the UART peripheral,
 *         the DMA stream and disables the DMA and UART interrupts
 */
static void _uart_msp_deinit(UART_HandleTypeDef *huart)
{
    rcc_periph_clk_disable((const void *)STDIO_UART_USARTx);

    stdio_uart_tx_pin_deinit();
    stdio_uart_rx_pin_deinit();

    _error = stdio_uart_dma_deinit(huart);

    HAL_NVIC_DisableIRQ(STDIO_UART_USARTx_IRQn);
}

/**
 * @brief  UART Transfer complete callback
 * @param  huart
 * @retval None
 * @note   This function is called by the HAL library
 *         when the DMA is finished transferring data
 */
static void _tx_cplt_callback(UART_HandleTypeDef *huart)
{
    portBASE_TYPE higher_priority_task_woken = pdFALSE;
    xSemaphoreGiveFromISR(_tx_cplt_semphr, &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

/**
 * @brief  UART Receive complete callback
 * @param  huart
 * @retval None
 * @note   This function is called by the HAL library
 *         when a character is arrived.
 */
static void _rx_cplt_callback(UART_HandleTypeDef *huart)
{
    portBASE_TYPE higher_priority_task_woken = pdFALSE;
    xQueueSendFromISR(_rx_queue, &_rx_buffer, &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

static void _uart_error_callback(UART_HandleTypeDef *huart)
{
    _error_handler();
}

static void _error_handler(void)
{
    if (HAL_UART_ERROR_ORE & HAL_UART_GetError(&h_stdio_uart))
    {
        __HAL_UART_CLEAR_OREFLAG(&h_stdio_uart);
    }
    else
    {
        //TODO: proper error handling
        while (1)
        {

        }
    }
}

static int _uart_write(const uint8_t *data, size_t len)
{
    const TickType_t ticks_to_wait = pdMS_TO_TICKS(2ul * dma_tx_max_time_ms);
    BaseType_t ret;
    uart_tx_data_t tx_data = {
        .pbuf = NULL,
        .size = 0,
    };

    ret = xQueueReceive(_tx_avail_queue, &tx_data.pbuf, ticks_to_wait);
    if (pdTRUE != ret)
    {
        return -ETIMEDOUT;
    }

    memcpy(tx_data.pbuf, (uint8_t *)data, len);
    tx_data.size = len;

    ret = xQueueSend(_tx_ready_queue, &tx_data, ticks_to_wait);
    if (pdTRUE != ret)
    {
        return -ETIMEDOUT;
    }

    return 0;
}

/**
 * @brief  Locks the stdin mutex
 */
static inline void stdin_lock(void)
{
    xSemaphoreTake(_stdin_mutex, portMAX_DELAY);
}

/**
 * @brief  Unlocks the stdin mutex
 */
static inline void stdin_unlock(void)
{
    xSemaphoreGive(_stdin_mutex);
}



