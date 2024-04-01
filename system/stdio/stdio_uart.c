/*
 * Original work Copyright (C) 2013 INRIA
 *                             2015 Kaspar Schleiser <kaspar@schleiser.de>
 *                             2016 Eistec AB
 *                             2018 Freie Universität Berlin
 * Modified work Copyright (C) 2024 Balint Kardos
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the root
 * directory of this software component or visit
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html for more details
 *
 * The original work can be found in the RIOT-OS 2024.01 branch:
 *     RIOT-OS repository: https://github.com/RIOT-OS/RIOT/tree/2024.01-branch
 *     stdio_uart.c:       https://github.com/RIOT-OS/RIOT/blob/2024.01-branch/sys/stdio_uart/stdio_uart.c
 *
 * The original authors of stdio_uart.c are:
 *     Oliver Hahm <oliver.hahm@inria.fr>
 *     Ludwig Knüpfer <ludwig.knuepfer@fu-berlin.de>
 *     Kaspar Schleiser <kaspar@schleiser.de>
 *     Joakim Nohlgård <joakim.nohlgard@eistec.se>
 *     Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

/**
 * @ingroup     system_stdio
 * @{
 *
 * @file        stdio_uart.c
 * @brief       STDIO over UART implementation
 *              This file implements UART callback and the STDIO read/write functions
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
static inline void _stdin_lock(void);
static inline void _stdin_unlock(void);
static int _uart_write(const uint8_t *data, size_t len);
static void _error_handler(void);
static void _uart_rtos_init(void);
static void _uart_rtos_deinit(void);
static int _uart_periph_init(void);
static int _uart_periph_deinit(void);

void stdio_init(void)
{
    _uart_rtos_init();
    _uart_periph_init();
}

void stdio_deinit(void)
{
    _uart_rtos_deinit();
    _uart_periph_deinit();
}

int stdio_add_stdin_listener(const QueueHandle_t hqueue)
{
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

ssize_t stdio_read(void *buffer, size_t max_len)
{
    uint8_t *buf = buffer;

    _stdin_lock();

    for (size_t i = 0; i < max_len; i++)
    {
        xQueueReceive(_stdin_queue, &buf[i], portMAX_DELAY);
    }

    _stdin_unlock();

    return max_len;
}

ssize_t stdio_write(const void *buffer, size_t len)
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

/**
 * @brief  UART writer gate-keeper task
 *
 * @param  params not used
 *
 * @return None
 *
 * @note   Task that manages the data transmission over UART
 *         using DMA
 */
static void _write_task(void *params)
{
    (void)params;

    HAL_StatusTypeDef hal_status;
    uint8_t *_tx_pending = NULL;

    uart_tx_data_t uart_tx_data = {
        .pbuf = NULL,
        .size = 0,
    };

    const TickType_t ticks_to_wait = pdMS_TO_TICKS(2ul * dma_tx_max_time_ms);

    for ( ;; )
    {
        xQueueReceive(_tx_ready_queue, &uart_tx_data, portMAX_DELAY);

        hal_status = HAL_UART_Transmit_DMA(&h_stdio_uart, uart_tx_data.pbuf, uart_tx_data.size);
        if (HAL_OK != hal_status)
        {
            _error_handler();
        }

        _tx_pending = uart_tx_data.pbuf;

        xSemaphoreTake(_tx_cplt_semphr, ticks_to_wait);
        xQueueSend(_tx_avail_queue, &_tx_pending, 0);
    }
}

/**
 * @brief  UART reader task
 *
 * @param  params not used
 *
 * @return None
 *
 * @note   Task that manages the data reception over UART
 */
static void _read_task(void *params)
{
    (void)params;

    HAL_StatusTypeDef hal_status = HAL_UART_Receive_IT(&h_stdio_uart, &_rx_buffer, 1);
    if (HAL_OK != hal_status)
    {
        _error_handler();
    }

    for ( ;; )
    {
        uint8_t rx_data;
        xQueueReceive(_rx_queue, &rx_data, portMAX_DELAY);

        hal_status = HAL_UART_Receive_IT(&h_stdio_uart, &_rx_buffer, 1);
        if (HAL_OK != hal_status)
        {
            _error_handler();
        }

        if (NULL != xSemaphoreGetMutexHolder(_stdin_mutex))
        {
            xQueueSend(_stdin_queue, &rx_data, portMAX_DELAY);
        }
        else
        {
            for (uint32_t i = 0; i < _stdin_listeners; i++)
            {
                xQueueSend(_stdin_listeners_list[i], &rx_data, portMAX_DELAY);
            }
        }
    }
}

/**
 * @brief  Initializes the STDIO UART RTOS related objects
 *
 * @param  None
 *
 * @return None
 *
 * @note   This function creates the UART reader / writer tasks and the
 *         corresponding queues, semaphores and mutexes
 * @note   stdio_uart_config.h contains necessary informations for
 *         task priority, task length, stack sizes, etc
 */
static void _uart_rtos_init(void)
{
    _stdin_listeners = 0ul;

    _tx_cplt_semphr = xSemaphoreCreateBinaryStatic(&_tx_cplt_semphr_storage);

    _tx_avail_queue = xQueueCreateStatic(STDIO_UART_TX_AVAIL_QUEUE_LENGTH,
                                         sizeof(uint8_t *),
                                         _tx_avail_queue_storage,
                                         &_tx_avail_queue_struct);

    _tx_ready_queue = xQueueCreateStatic(STDIO_UART_TX_READY_QUEUE_LENGTH,
                                         sizeof(uart_tx_data_t),
                                         _tx_ready_queue_storage,
                                         &_tx_ready_queue_struct);

    _rx_queue = xQueueCreateStatic(STDIO_UART_RX_QUEUE_LENGTH,
                                   sizeof(uint8_t),
                                   _rx_queue_storage,
                                   &_rx_queue_struct);

    _stdin_mutex = xSemaphoreCreateMutexStatic(&_stdin_mutex_storage);

    _stdin_queue = xQueueCreateStatic(STDIO_UART_STDIN_QUEUE_LENGTH,
                                      sizeof(uint8_t),
                                      _stdin_queue_storage,
                                      &_stdin_queue_struct);

    for (uint8_t i = 0; i < STDIO_UART_TX_AVAIL_QUEUE_LENGTH; i++)
    {
        uint8_t *buffer_address = &_tx_buffer[i * STDIO_UART_TX_BUFFER_DEPTH];
        xQueueSend(_tx_avail_queue, &buffer_address, 0);
    }

    h_write_task = xTaskCreateStatic(_write_task,
                                     "STDIO UART Write",
                                     STDIO_UART_WRITE_TASK_STACKSIZE,
                                     NULL,
                                     STDIO_UART_WRITE_TASK_PRIORITY,
                                     _write_task_stack,
                                     &_write_task_tcb);

    h_read_task = xTaskCreateStatic(_read_task,
                                    "STDIO UART Read",
                                    STDIO_UART_READ_TASK_STACKSIZE,
                                    NULL,
                                    STDIO_UART_READ_TASK_PRIORITY,
                                    _read_task_stack,
                                    &_read_task_tcb);
}

/**
 * @brief  Initializes the STDIO UART peripheral (high-level)
 *
 * @param  None
 *
 * @return 0 on success
 * @return < 0 on error
 *
 * @note   This function initializes the STDIO UART peripheral based on
 *         the hardware configuration defined in stdio_uart_config.h
 */
static int _uart_periph_init(void)
{
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
 * @brief  De-initializes the STDIO UART RTOS related objects
 *
 * @param  None
 *
 * @return None
 *
 * @note   This function deletes the UART reader / writer tasks and the
 *         corresponding queues, semaphores and mutexes
 */
static void _uart_rtos_deinit(void)
{
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
}

/**
 * @brief  De-initializes the STDIO UART peripheral (high-level)
 *
 * @param  None
 *
 * @return 0 on success
 * @return < 0 on error
 */
static int _uart_periph_deinit(void)
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

    return 0;
}

/**
 * @brief     Initializes the STDIO_UART MSP (low-level)
 *
 * @param[in] huart pointer to a UART_HandleTypeDef structure that contains
 *            the configuration information for the stdio UART peripheral
 *
 * @return    None
 *
 * @note      This function initializes the GPIOs corresponding the UART peripheral,
 *            the DMA stream and enables the DMA and UART interrupts and the peripheral
 *            clocks
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
 * @brief     De-initializes the STDIO_UART MSP (low-level)
 *
 * @param[in] huart pointer to a UART_HandleTypeDef structure that contains
 *            the configuration information for the stdio UART peripheral
 *
 * @return    None
 *
 * @note      This function de-initializes the GPIOs corresponding the UART peripheral,
 *            the DMA stream and disables the DMA and UART interrupts and the peripheral
 *            clocks if possible
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
 * @brief     UART Transfer complete callback
 *
 * @param[in] huart pointer to a UART_HandleTypeDef structure that contains
 *            the configuration information for the stdio UART peripheral
 *
 * @return    None
 *
 * @note      This function is called by the HAL library
 *            when the DMA is finished transferring data
 */
static void _tx_cplt_callback(UART_HandleTypeDef *huart)
{
    portBASE_TYPE higher_priority_task_woken = pdFALSE;
    xSemaphoreGiveFromISR(_tx_cplt_semphr, &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

/**
 * @brief     UART Receive complete callback
 *
 * @param[in] huart pointer to a UART_HandleTypeDef structure that contains
 *            the configuration information for the stdio UART peripheral
 *
 * @return    None
 *
 * @note      This function is called by the HAL library
 *            when a character is arrived.
 */
static void _rx_cplt_callback(UART_HandleTypeDef *huart)
{
    portBASE_TYPE higher_priority_task_woken = pdFALSE;
    xQueueSendFromISR(_rx_queue, &_rx_buffer, &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

/**
 * @brief     UART Error callback
 *
 * @param[in] huart pointer to a UART_HandleTypeDef structure that contains
 *            the configuration information for the stdio UART peripheral
 *
 * @return    None
 *
 * @note      This function is called when the UART peripheral generates an error interrupt
 */
static void _uart_error_callback(UART_HandleTypeDef *huart)
{
    _error_handler();
}

/**
 * @brief  STDIO UART Error Handler
 *
 * @param  None
 *
 * @return None
 *
 * @note   This function is called when the UART peripheral encounters an error condition
 *         In case the ORE bit is set then the ORE bit is cleared and the peripheral
 *         continues to operate
 *         In any other error case the peripheral is de-initialized and hangs in an
 *         infinite loop
 */
static void _error_handler(void)
{
    if (HAL_UART_ERROR_ORE & HAL_UART_GetError(&h_stdio_uart))
    {
        __HAL_UART_CLEAR_OREFLAG(&h_stdio_uart);
    }
    else
    {
        _uart_periph_deinit();
        _uart_periph_init();

        while (1)
        {

        }
    }
}

/**
 * @brief     Sends the formatted data over UART
 *
 * @param[in] data pointer to the data buffer
 * @param[in] len the size of the data in bytes
 *
 * @return    0 on success
 * @return    < 0 on error
 *
 * @note      This function only sends the data to the tx ready queue
 *            If the queue is full than this function will block the caller task
 */
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
static inline void _stdin_lock(void)
{
    xSemaphoreTake(_stdin_mutex, portMAX_DELAY);
}

/**
 * @brief  Unlocks the stdin mutex
 */
static inline void _stdin_unlock(void)
{
    xSemaphoreGive(_stdin_mutex);
}

/**
 * @brief This function handles the STDIO USART global interrupt.
 */
void STDIO_UART_IRQHandler(void)
{
    HAL_UART_IRQHandler(&h_stdio_uart);
}

