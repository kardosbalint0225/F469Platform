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

#include "stdio_uart.h"
#include "stdio_uart_config.h"
#include "stm32f4xx_hal.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

typedef struct
{
    uint8_t *pbuf;
    uint16_t size;
} uart_tx_data_t;

static SemaphoreHandle_t h_uart_tx_cplt_semphr = NULL;
static StaticSemaphore_t uart_tx_cplt_semphr_storage;

static StaticQueue_t uart_rx_queue_struct;
static uint8_t uart_rx_queue_storage[STDIO_UART_RX_QUEUE_LENGTH * sizeof(uint8_t)];
QueueHandle_t h_uart_rx_queue = NULL;

static StaticQueue_t uart_tx_avail_struct;
static uint8_t uart_tx_avail_queue_storage[STDIO_UART_TX_AVAIL_QUEUE_LENGTH * sizeof(uint8_t *)];
static QueueHandle_t h_uart_tx_avail_queue = NULL;

static StaticQueue_t uart_tx_ready_struct;
static uint8_t uart_tx_ready_queue_storage[STDIO_UART_TX_READY_QUEUE_LENGTH * sizeof(uart_tx_data_t)];
static QueueHandle_t h_uart_tx_ready_queue = NULL;

static StackType_t stdio_uart_write_task_stack[STDIO_UART_WRITE_TASK_STACKSIZE];
static StaticTask_t stdio_uart_write_task_tcb;
static TaskHandle_t h_stdio_uart_write_task = NULL;

UART_HandleTypeDef h_stdio_uart;
DMA_HandleTypeDef h_stdio_uart_dma_tx;

static uint8_t uart_tx_buffer[STDIO_UART_TX_BUFFER_DEPTH * STDIO_UART_TX_AVAIL_QUEUE_LENGTH];
static uint8_t uart_rx_buffer;

static const TickType_t dma_tx_max_time_ms = (TickType_t)((1000.0f * (((float)(10 * STDIO_UART_TX_BUFFER_DEPTH)) / 115200.0f)) + 0.5f);

static uint32_t stdio_uart_error;

static void uartx_init(void);
static void uartx_deinit(void);
static void uartx_msp_init(UART_HandleTypeDef *huart);
static void uartx_msp_deinit(UART_HandleTypeDef *huart);
static void uartx_tx_cplt_callback(UART_HandleTypeDef *huart);
static void uartx_rx_cplt_callback(UART_HandleTypeDef *huart);

static void stdio_uart_write_task(void *params);

/**
 * @brief  UART writer gatekeeper task
 * @param  params not used
 * @retval None
 * @note   Task that performs the UART TX related jobs.
 */
static void stdio_uart_write_task(void *params)
{
    (void)params;

    BaseType_t ret;
    HAL_StatusTypeDef hal_status;
    uint8_t *uart_tx_pending = NULL;

    uart_tx_data_t uart_tx_data = {
        .pbuf = NULL,
        .size = 0,
    };

    const TickType_t ticks_to_wait = pdMS_TO_TICKS(2UL * dma_tx_max_time_ms);

    for ( ;; )
    {
        ret = xQueueReceive(h_uart_tx_ready_queue, &uart_tx_data, portMAX_DELAY);
        stdio_uart_error |= (pdPASS != ret) ? STDIO_UART_ERROR_TX_READY_QUEUE_RECEIVE : 0UL;
        assert_param(0UL == stdio_uart_error);

        hal_status = HAL_UART_Transmit_DMA(&h_stdio_uart, uart_tx_data.pbuf, uart_tx_data.size);
        stdio_uart_error |= (HAL_OK != hal_status) ? STDIO_UART_ERROR_UART_TRANSMIT_DMA : 0UL;
        assert_param(0UL == stdio_uart_error);
        uart_tx_pending = uart_tx_data.pbuf;

        ret = xSemaphoreTake(h_uart_tx_cplt_semphr, ticks_to_wait);
        stdio_uart_error |= (pdPASS != ret) ? STDIO_UART_ERROR_TX_CPLT_SEMPHR_TAKE : 0UL;
        assert_param(0UL == stdio_uart_error);

        ret = xQueueSend(h_uart_tx_avail_queue, &uart_tx_pending, 0);
        stdio_uart_error |= (pdPASS != ret) ? STDIO_UART_ERROR_TX_AVAIL_QUEUE_SEND : 0UL;
        assert_param(0UL == stdio_uart_error);
    }
}

void stdio_init(void)
{
    BaseType_t ret;
    stdio_uart_error = 0UL;

    uartx_init();

    h_uart_tx_cplt_semphr = xSemaphoreCreateBinaryStatic(&uart_tx_cplt_semphr_storage);
    stdio_uart_error |= (NULL == h_uart_tx_cplt_semphr) ? STDIO_UART_ERROR_TX_CPLT_SEMPHR_CREATE : 0UL;
    assert_param(0UL == stdio_uart_error);

    h_uart_tx_avail_queue = xQueueCreateStatic(STDIO_UART_TX_AVAIL_QUEUE_LENGTH,
                                               sizeof(uint8_t *),
                                               uart_tx_avail_queue_storage,
                                               &uart_tx_avail_struct);
    stdio_uart_error |= (NULL == h_uart_tx_avail_queue) ? STDIO_UART_ERROR_TX_AVAIL_QUEUE_CREATE : 0UL;
    assert_param(0UL == stdio_uart_error);

    h_uart_tx_ready_queue = xQueueCreateStatic(STDIO_UART_TX_READY_QUEUE_LENGTH,
                                               sizeof(uart_tx_data_t),
                                               uart_tx_ready_queue_storage,
                                               &uart_tx_ready_struct);
    stdio_uart_error |= (NULL == h_uart_tx_ready_queue) ? STDIO_UART_ERROR_TX_READY_QUEUE_CREATE : 0UL;
    assert_param(0UL == stdio_uart_error);

    h_uart_rx_queue = xQueueCreateStatic(STDIO_UART_RX_QUEUE_LENGTH,
                                         sizeof(uint8_t),
                                         uart_rx_queue_storage,
                                         &uart_rx_queue_struct);
    stdio_uart_error |= (NULL == h_uart_rx_queue) ? STDIO_UART_ERROR_RX_QUEUE_CREATE : 0UL;
    assert_param(0UL == stdio_uart_error);

    for (uint8_t i = 0; i < STDIO_UART_TX_AVAIL_QUEUE_LENGTH; i++)
    {
        uint8_t *buffer_address = &uart_tx_buffer[i * STDIO_UART_TX_BUFFER_DEPTH];
        ret = xQueueSend(h_uart_tx_avail_queue, &buffer_address, 0);
        stdio_uart_error |= (pdPASS != ret) ? STDIO_UART_ERROR_TX_AVAIL_QUEUE_INIT : 0UL;
        assert_param(0UL == stdio_uart_error);
    }

    h_stdio_uart_write_task = xTaskCreateStatic(stdio_uart_write_task,
                                                "UART Write",
                                                STDIO_UART_WRITE_TASK_STACKSIZE,
                                                NULL,
                                                STDIO_UART_WRITE_TASK_PRIORITY,
                                                stdio_uart_write_task_stack,
                                                &stdio_uart_write_task_tcb);
    stdio_uart_error |= (NULL == h_stdio_uart_write_task) ? STDIO_UART_ERROR_WRITE_TASK_CREATE : 0UL;
    assert_param(0UL == stdio_uart_error);

    HAL_StatusTypeDef hal_status = HAL_UART_Receive_IT(&h_stdio_uart, &uart_rx_buffer, 1);
    stdio_uart_error |= (HAL_OK != hal_status) ? STDIO_UART_ERROR_UART_RECEIVE_IT : 0UL;
    assert_param(0UL == stdio_uart_error);
}

void stdio_deinit(void)
{
    uartx_deinit();

    vTaskDelete(h_stdio_uart_write_task);
    vQueueDelete(h_uart_tx_avail_queue);
    vQueueDelete(h_uart_tx_ready_queue);
    vQueueDelete(h_uart_rx_queue);
    vSemaphoreDelete(h_uart_tx_cplt_semphr);

    h_stdio_uart_write_task = NULL;
    h_uart_tx_avail_queue = NULL;
    h_uart_tx_ready_queue = NULL;
    h_uart_rx_queue = NULL;
    h_uart_tx_cplt_semphr = NULL;
}

/**
 * @brief  Initializes the UART peripheral
 * @param  None
 * @retval None
 * @note   The communication is configured 115200 Baudrate 8N1 with no
 *         flowcontrol.
 */
static void uartx_init(void)
{
    h_stdio_uart.Instance = STDIO_UART_USARTx;
    h_stdio_uart.Init.BaudRate = 115200;
    h_stdio_uart.Init.WordLength = UART_WORDLENGTH_8B;
    h_stdio_uart.Init.StopBits = UART_STOPBITS_1;
    h_stdio_uart.Init.Parity = UART_PARITY_NONE;
    h_stdio_uart.Init.Mode = UART_MODE_TX_RX;
    h_stdio_uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    h_stdio_uart.Init.OverSampling = UART_OVERSAMPLING_16;

    HAL_StatusTypeDef ret;

    ret = HAL_UART_RegisterCallback(&h_stdio_uart, HAL_UART_MSPINIT_CB_ID, uartx_msp_init);
    stdio_uart_error |= (HAL_OK != ret) ? STDIO_UART_ERROR_REGISTER_MSPINIT_CB : 0UL;
    assert_param(0UL == stdio_uart_error);

    ret = HAL_UART_RegisterCallback(&h_stdio_uart, HAL_UART_MSPDEINIT_CB_ID, uartx_msp_deinit);
    stdio_uart_error |= (HAL_OK != ret) ? STDIO_UART_ERROR_REGISTER_MSPDEINIT_CB : 0UL;
    assert_param(0UL == stdio_uart_error);

    ret = HAL_UART_Init(&h_stdio_uart);
    stdio_uart_error |= (HAL_OK != ret) ? STDIO_UART_ERROR_UART_INIT : 0UL;
    assert_param(0UL == stdio_uart_error);

    ret = HAL_UART_RegisterCallback(&h_stdio_uart, HAL_UART_TX_COMPLETE_CB_ID, uartx_tx_cplt_callback);
    stdio_uart_error |= (HAL_OK != ret) ? STDIO_UART_ERROR_REGISTER_TX_CPLT_CB : 0UL;
    assert_param(0UL == stdio_uart_error);

    ret = HAL_UART_RegisterCallback(&h_stdio_uart, HAL_UART_RX_COMPLETE_CB_ID, uartx_rx_cplt_callback);
    stdio_uart_error |= (HAL_OK != ret) ? STDIO_UART_ERROR_REGISTER_RX_CPLT_CB : 0UL;
    assert_param(0UL == stdio_uart_error);

    //TODO: Error callback
}

/**
 * @brief  Initializes the UARTx MSP
 * @param  huart pointer to the UART_HandleTypeDef structure
 * @retval None
 * @note   This function initializes the GPIOs corresponding the UART peripheral,
 *         the DMA stream and enables the DMA and UART interrupts
 */
static void uartx_msp_init(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    STDIO_UART_USARTx_CLK_ENABLE();
    STDIO_UART_DMAx_CLK_ENABLE();
    STDIO_UART_GPIOx_CLK_ENABLE();

    GPIO_InitStruct.Pin = STDIO_UART_UARTx_TX_PIN | STDIO_UART_UARTx_RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = STDIO_UART_GPIO_AFx_USARTx;
    HAL_GPIO_Init(STDIO_UART_GPIOx_PORT, &GPIO_InitStruct);

    h_stdio_uart_dma_tx.Instance = STDIO_UART_DMAx_STREAMx;
    h_stdio_uart_dma_tx.Init.Channel = STDIO_UART_DMA_CHANNELx;
    h_stdio_uart_dma_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    h_stdio_uart_dma_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    h_stdio_uart_dma_tx.Init.MemInc = DMA_MINC_ENABLE;
    h_stdio_uart_dma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    h_stdio_uart_dma_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    h_stdio_uart_dma_tx.Init.Mode = DMA_NORMAL;
    h_stdio_uart_dma_tx.Init.Priority = DMA_PRIORITY_LOW;
    h_stdio_uart_dma_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    HAL_StatusTypeDef ret = HAL_DMA_Init(&h_stdio_uart_dma_tx);
    stdio_uart_error |= (HAL_OK != ret) ? STDIO_UART_ERROR_DMA_INIT : 0UL;
    assert_param(0UL == stdio_uart_error);

    __HAL_LINKDMA(huart, hdmatx, h_stdio_uart_dma_tx);

    HAL_NVIC_SetPriority(STDIO_UART_USARTx_IRQn, STDIO_UART_USARTx_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(STDIO_UART_USARTx_IRQn);

    HAL_NVIC_SetPriority(STDIO_UART_DMAx_STREAMx_IRQn, STDIO_UART_DMAx_STREAMx_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(STDIO_UART_DMAx_STREAMx_IRQn);
}

/**
 * @brief  Deinitializes the UART peripheral
 * @param  None
 * @retval None
 * @note   -
 */
static void uartx_deinit(void)
{
    HAL_StatusTypeDef ret;

    ret = HAL_UART_UnRegisterCallback(&h_stdio_uart, HAL_UART_TX_COMPLETE_CB_ID);
    stdio_uart_error |= (HAL_OK != ret) ? STDIO_UART_ERROR_UNREGISTER_TX_CPLT_CB : 0UL;
    assert_param(0UL == stdio_uart_error);

    ret = HAL_UART_UnRegisterCallback(&h_stdio_uart, HAL_UART_RX_COMPLETE_CB_ID);
    stdio_uart_error |= (HAL_OK != ret) ? STDIO_UART_ERROR_UNREGISTER_RX_CPLT_CB : 0UL;
    assert_param(0UL == stdio_uart_error);

    ret = HAL_UART_DeInit(&h_stdio_uart);
    stdio_uart_error |= (HAL_OK != ret) ? STDIO_UART_ERROR_UART_DEINIT : 0UL;
    assert_param(0UL == stdio_uart_error);

    ret = HAL_UART_UnRegisterCallback(&h_stdio_uart, HAL_UART_MSPINIT_CB_ID);
    stdio_uart_error |= (HAL_OK != ret) ? STDIO_UART_ERROR_UNREGISTER_MSPINIT_CB : 0UL;
    assert_param(0UL == stdio_uart_error);

    ret = HAL_UART_UnRegisterCallback(&h_stdio_uart, HAL_UART_MSPDEINIT_CB_ID);
    stdio_uart_error |= (HAL_OK != ret) ? STDIO_UART_ERROR_UNREGISTER_MSPDEINIT_CB : 0UL;
    assert_param(0UL == stdio_uart_error);
}

/**
 * @brief  Deinitializes the UARTx MSP
 * @param  huart pointer to the UART_HandleTypeDef structure
 * @retval None
 * @note   This function deinitializes the GPIOs corresponding the UART peripheral,
 *         the DMA stream and disables the DMA and UART interrupts
 */
static void uartx_msp_deinit(UART_HandleTypeDef *huart)
{
    STDIO_UART_USARTx_CLK_DISABLE();

    HAL_GPIO_DeInit(STDIO_UART_GPIOx_PORT, STDIO_UART_UARTx_TX_PIN | STDIO_UART_UARTx_RX_PIN);

    HAL_StatusTypeDef ret;
    ret = HAL_DMA_DeInit(huart->hdmatx);
    stdio_uart_error |= (HAL_OK != ret) ? STDIO_UART_ERROR_DMA_DEINIT : 0UL;
    assert_param(0UL == stdio_uart_error);

    HAL_NVIC_DisableIRQ(STDIO_UART_USARTx_IRQn);
    HAL_NVIC_DisableIRQ(STDIO_UART_DMAx_STREAMx_IRQn);
}

uint32_t stdio_uart_get_error(void)
{
    return stdio_uart_error;
}

/**
 * @brief  UART Transfer complete callback
 * @param  huart
 * @retval None
 * @note   This function is called by the HAL library
 *         when the DMA is finished transferring data
 */
static void uartx_tx_cplt_callback(UART_HandleTypeDef *huart)
{
    portBASE_TYPE higher_priority_task_woken = pdFALSE;
    xSemaphoreGiveFromISR(h_uart_tx_cplt_semphr, &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

/**
 * @brief  UART Receive complete callback
 * @param  huart
 * @retval None
 * @note   This function is called by the HAL library
 *         when a character is arrived.
 */
static void uartx_rx_cplt_callback(UART_HandleTypeDef *huart)
{
    portBASE_TYPE higher_priority_task_woken = pdFALSE;
    xQueueSendFromISR(h_uart_rx_queue, &uart_rx_buffer, &higher_priority_task_woken);
    HAL_UART_Receive_IT(&h_stdio_uart, &uart_rx_buffer, 1);
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

int stdio_available(void)
{
    return -ENOTSUP;
}

ssize_t stdio_read(void* buffer, size_t count)
{
    return -ENOTSUP;
}

ssize_t stdio_write(const void *buffer, size_t len)
{
    ssize_t result = len;
    uart_tx_data_t data = {
        .pbuf = NULL,
        .size = 0,
    };

    BaseType_t ret;
    const TickType_t ticks_to_wait = pdMS_TO_TICKS(2UL * dma_tx_max_time_ms);

    ret = xQueueReceive(h_uart_tx_avail_queue, &data.pbuf, ticks_to_wait);
    stdio_uart_error |= (pdPASS != ret) ? STDIO_UART_ERROR_TX_AVAIL_QUEUE_RECEIVE : 0UL;
    assert_param(0UL == stdio_uart_error);

    memcpy(data.pbuf, (uint8_t *)buffer, len);
    data.size = len;

    ret = xQueueSend(h_uart_tx_ready_queue, &data, ticks_to_wait);
    stdio_uart_error |= (pdPASS != ret) ? STDIO_UART_ERROR_TX_READY_QUEUE_SEND : 0UL;
    assert_param(0UL == stdio_uart_error);
    return result;
}

