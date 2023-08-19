/*
 * uart_console.c
 *
 *  Created on: 2023. jul. 9.
 *      Author: Balint
 */
#include "uart_console.h"
#include "uart_console_config.h"
#include "stm32f4xx_hal.h"

#include <string.h>

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
static uint8_t uart_rx_queue_storage[UART_CONSOLE_RX_QUEUE_LENGTH * sizeof(uint8_t)];
QueueHandle_t h_uart_rx_queue = NULL;

static StaticQueue_t uart_tx_avail_struct;
static uint8_t uart_tx_avail_queue_storage[UART_CONSOLE_TX_AVAIL_QUEUE_LENGTH * sizeof(uint8_t *)];
static QueueHandle_t h_uart_tx_avail_queue = NULL;

static StaticQueue_t uart_tx_ready_struct;
static uint8_t uart_tx_ready_queue_storage[UART_CONSOLE_TX_READY_QUEUE_LENGTH * sizeof(uart_tx_data_t)];
static QueueHandle_t h_uart_tx_ready_queue = NULL;

static StackType_t uart_console_write_task_stack[UART_CONSOLE_WRITE_TASK_STACKSIZE];
static StaticTask_t uart_console_write_task_tcb;
static TaskHandle_t h_uart_console_write_task = NULL;

UART_HandleTypeDef h_uart_console;
DMA_HandleTypeDef h_dma_uart_console_tx;

static uint8_t uart_tx_buffer[UART_CONSOLE_TX_BUFFER_DEPTH * UART_CONSOLE_TX_AVAIL_QUEUE_LENGTH];
static uint8_t uart_rx_buffer;

static const TickType_t dma_tx_max_time_ms = (TickType_t)((1000.0f * (((float)(10 * UART_CONSOLE_TX_BUFFER_DEPTH)) / 115200.0f)) + 0.5f);

static uint32_t uart_console_error;

static void uartx_init(void);
static void uartx_deinit(void);
static void uartx_msp_init(UART_HandleTypeDef *huart);
static void uartx_msp_deinit(UART_HandleTypeDef *huart);
static void uartx_tx_cplt_callback(UART_HandleTypeDef *huart);
static void uartx_rx_cplt_callback(UART_HandleTypeDef *huart);

/**
 * @brief  UART writer gatekeeper task
 * @param  params not used
 * @retval None
 * @note   Task that performs the UART TX related jobs.
 */
static void uart_console_write_task(void *params)
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
        uart_console_error |= (pdPASS != ret) ? UART_CONSOLE_ERROR_TX_READY_QUEUE_RECEIVE : 0UL;
        assert_param(0UL == uart_console_error);

        hal_status = HAL_UART_Transmit_DMA(&h_uart_console, uart_tx_data.pbuf, uart_tx_data.size);
        uart_console_error |= (HAL_OK != hal_status) ? UART_CONSOLE_ERROR_UART_TRANSMIT_DMA : 0UL;
        assert_param(0UL == uart_console_error);
        uart_tx_pending = uart_tx_data.pbuf;

        ret = xSemaphoreTake(h_uart_tx_cplt_semphr, ticks_to_wait);
        uart_console_error |= (pdPASS != ret) ? UART_CONSOLE_ERROR_TX_CPLT_SEMPHR_TAKE : 0UL;
        assert_param(0UL == uart_console_error);

        ret = xQueueSend(h_uart_tx_avail_queue, &uart_tx_pending, 0);
        uart_console_error |= (pdPASS != ret) ? UART_CONSOLE_ERROR_TX_AVAIL_QUEUE_SEND : 0UL;
        assert_param(0UL == uart_console_error);
    }
}

/**
 * @brief  Initializes the UART Console
 * @param  None
 * @retval None
 * @note   This function initializes the UART peripheral,
 *         creates the UART writer task, queues and semaphores
 */
void uart_console_init(void)
{
    BaseType_t ret;
    uart_console_error = 0UL;

    uartx_init();

    h_uart_tx_cplt_semphr = xSemaphoreCreateBinaryStatic(&uart_tx_cplt_semphr_storage);
    uart_console_error |= (NULL == h_uart_tx_cplt_semphr) ? UART_CONSOLE_ERROR_TX_CPLT_SEMPHR_CREATE : 0UL;
    assert_param(0UL == uart_console_error);

    h_uart_tx_avail_queue = xQueueCreateStatic(UART_CONSOLE_TX_AVAIL_QUEUE_LENGTH,
                                               sizeof(uint8_t *),
                                               uart_tx_avail_queue_storage,
                                               &uart_tx_avail_struct);
    uart_console_error |= (NULL == h_uart_tx_avail_queue) ? UART_CONSOLE_ERROR_TX_AVAIL_QUEUE_CREATE : 0UL;
    assert_param(0UL == uart_console_error);

    h_uart_tx_ready_queue = xQueueCreateStatic(UART_CONSOLE_TX_READY_QUEUE_LENGTH,
                                               sizeof(uart_tx_data_t),
                                               uart_tx_ready_queue_storage,
                                               &uart_tx_ready_struct);
    uart_console_error |= (NULL == h_uart_tx_ready_queue) ? UART_CONSOLE_ERROR_TX_READY_QUEUE_CREATE : 0UL;
    assert_param(0UL == uart_console_error);

    h_uart_rx_queue = xQueueCreateStatic(UART_CONSOLE_RX_QUEUE_LENGTH,
                                         sizeof(uint8_t),
                                         uart_rx_queue_storage,
                                         &uart_rx_queue_struct);
    uart_console_error |= (NULL == h_uart_rx_queue) ? UART_CONSOLE_ERROR_RX_QUEUE_CREATE : 0UL;
    assert_param(0UL == uart_console_error);

    for (uint8_t i = 0; i < UART_CONSOLE_TX_AVAIL_QUEUE_LENGTH; i++)
    {
        uint8_t *buffer_address = &uart_tx_buffer[i * UART_CONSOLE_TX_BUFFER_DEPTH];
        ret = xQueueSend(h_uart_tx_avail_queue, &buffer_address, 0);
        uart_console_error |= (pdPASS != ret) ? UART_CONSOLE_ERROR_TX_AVAIL_QUEUE_INIT : 0UL;
        assert_param(0UL == uart_console_error);
    }

    h_uart_console_write_task = xTaskCreateStatic(uart_console_write_task,
                                                  "UART Write",
                                                  UART_CONSOLE_WRITE_TASK_STACKSIZE,
                                                  NULL,
                                                  UART_CONSOLE_WRITE_TASK_PRIORITY,
                                                  uart_console_write_task_stack,
                                                  &uart_console_write_task_tcb);
    uart_console_error |= (NULL == h_uart_console_write_task) ? UART_CONSOLE_ERROR_WRITE_TASK_CREATE : 0UL;
    assert_param(0UL == uart_console_error);

    HAL_StatusTypeDef hal_status = HAL_UART_Receive_IT(&h_uart_console, &uart_rx_buffer, 1);
    uart_console_error |= (HAL_OK != hal_status) ? UART_CONSOLE_ERROR_UART_RECEIVE_IT : 0UL;
    assert_param(0UL == uart_console_error);
}

/**
 * @brief  Deinitializes the UART Console
 * @param  None
 * @retval None
 * @note   This function deinitializes the UART peripheral
 *         and deletes the UART writer task, queues and semaphores
 */
void uart_console_deinit(void)
{
    uartx_deinit();

    vTaskDelete(h_uart_console_write_task);
    vQueueDelete(h_uart_tx_avail_queue);
    vQueueDelete(h_uart_tx_ready_queue);
    vQueueDelete(h_uart_rx_queue);
    vSemaphoreDelete(h_uart_tx_cplt_semphr);

    h_uart_console_write_task = NULL;
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
    h_uart_console.Instance = UART_CONSOLE_USARTx;
    h_uart_console.Init.BaudRate = 115200;
    h_uart_console.Init.WordLength = UART_WORDLENGTH_8B;
    h_uart_console.Init.StopBits = UART_STOPBITS_1;
    h_uart_console.Init.Parity = UART_PARITY_NONE;
    h_uart_console.Init.Mode = UART_MODE_TX_RX;
    h_uart_console.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    h_uart_console.Init.OverSampling = UART_OVERSAMPLING_16;

    HAL_StatusTypeDef ret;

    ret = HAL_UART_RegisterCallback(&h_uart_console, HAL_UART_MSPINIT_CB_ID, uartx_msp_init);
    uart_console_error |= (HAL_OK != ret) ? UART_CONSOLE_ERROR_REGISTER_MSPINIT_CB : 0UL;
    assert_param(0UL == uart_console_error);

    ret = HAL_UART_RegisterCallback(&h_uart_console, HAL_UART_MSPDEINIT_CB_ID, uartx_msp_deinit);
    uart_console_error |= (HAL_OK != ret) ? UART_CONSOLE_ERROR_REGISTER_MSPDEINIT_CB : 0UL;
    assert_param(0UL == uart_console_error);

    ret = HAL_UART_Init(&h_uart_console);
    uart_console_error |= (HAL_OK != ret) ? UART_CONSOLE_ERROR_UART_INIT : 0UL;
    assert_param(0UL == uart_console_error);

    ret = HAL_UART_RegisterCallback(&h_uart_console, HAL_UART_TX_COMPLETE_CB_ID, uartx_tx_cplt_callback);
    uart_console_error |= (HAL_OK != ret) ? UART_CONSOLE_ERROR_REGISTER_TX_CPLT_CB : 0UL;
    assert_param(0UL == uart_console_error);

    ret = HAL_UART_RegisterCallback(&h_uart_console, HAL_UART_RX_COMPLETE_CB_ID, uartx_rx_cplt_callback);
    uart_console_error |= (HAL_OK != ret) ? UART_CONSOLE_ERROR_REGISTER_RX_CPLT_CB : 0UL;
    assert_param(0UL == uart_console_error);
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

    UART_CONSOLE_USARTx_CLK_ENABLE();
    UART_CONSOLE_DMAx_CLK_ENABLE();
    UART_CONSOLE_GPIOx_CLK_ENABLE();

    GPIO_InitStruct.Pin = UART_CONSOLE_UARTx_TX_PIN | UART_CONSOLE_UARTx_RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = UART_CONSOLE_GPIO_AFx_USARTx;
    HAL_GPIO_Init(UART_CONSOLE_GPIOx_PORT, &GPIO_InitStruct);

    h_dma_uart_console_tx.Instance = UART_CONSOLE_DMAx_STREAMx;
    h_dma_uart_console_tx.Init.Channel = UART_CONSOLE_DMA_CHANNELx;
    h_dma_uart_console_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    h_dma_uart_console_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    h_dma_uart_console_tx.Init.MemInc = DMA_MINC_ENABLE;
    h_dma_uart_console_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    h_dma_uart_console_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    h_dma_uart_console_tx.Init.Mode = DMA_NORMAL;
    h_dma_uart_console_tx.Init.Priority = DMA_PRIORITY_LOW;
    h_dma_uart_console_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    HAL_StatusTypeDef ret = HAL_DMA_Init(&h_dma_uart_console_tx);
    uart_console_error |= (HAL_OK != ret) ? UART_CONSOLE_ERROR_DMA_INIT : 0UL;
    assert_param(0UL == uart_console_error);

    __HAL_LINKDMA(huart, hdmatx, h_dma_uart_console_tx);

    HAL_NVIC_SetPriority(UART_CONSOLE_USARTx_IRQn, UART_CONSOLE_USARTx_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(UART_CONSOLE_USARTx_IRQn);

    HAL_NVIC_SetPriority(UART_CONSOLE_DMAx_STREAMx_IRQn, UART_CONSOLE_DMAx_STREAMx_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(UART_CONSOLE_DMAx_STREAMx_IRQn);
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

    ret = HAL_UART_UnRegisterCallback(&h_uart_console, HAL_UART_TX_COMPLETE_CB_ID);
    uart_console_error |= (HAL_OK != ret) ? UART_CONSOLE_ERROR_UNREGISTER_TX_CPLT_CB : 0UL;
    assert_param(0UL == uart_console_error);

    ret = HAL_UART_UnRegisterCallback(&h_uart_console, HAL_UART_RX_COMPLETE_CB_ID);
    uart_console_error |= (HAL_OK != ret) ? UART_CONSOLE_ERROR_UNREGISTER_RX_CPLT_CB : 0UL;
    assert_param(0UL == uart_console_error);

    ret = HAL_UART_DeInit(&h_uart_console);
    uart_console_error |= (HAL_OK != ret) ? UART_CONSOLE_ERROR_UART_DEINIT : 0UL;
    assert_param(0UL == uart_console_error);

    ret = HAL_UART_UnRegisterCallback(&h_uart_console, HAL_UART_MSPINIT_CB_ID);
    uart_console_error |= (HAL_OK != ret) ? UART_CONSOLE_ERROR_UNREGISTER_MSPINIT_CB : 0UL;
    assert_param(0UL == uart_console_error);

    ret = HAL_UART_UnRegisterCallback(&h_uart_console, HAL_UART_MSPDEINIT_CB_ID);
    uart_console_error |= (HAL_OK != ret) ? UART_CONSOLE_ERROR_UNREGISTER_MSPDEINIT_CB : 0UL;
    assert_param(0UL == uart_console_error);
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
    UART_CONSOLE_USARTx_CLK_DISABLE();

    HAL_GPIO_DeInit(UART_CONSOLE_GPIOx_PORT, UART_CONSOLE_UARTx_TX_PIN | UART_CONSOLE_UARTx_RX_PIN);

    HAL_StatusTypeDef ret;
    ret = HAL_DMA_DeInit(huart->hdmatx);
    uart_console_error |= (HAL_OK != ret) ? UART_CONSOLE_ERROR_DMA_DEINIT : 0UL;
    assert_param(0UL == uart_console_error);

    HAL_NVIC_DisableIRQ(UART_CONSOLE_USARTx_IRQn);
    HAL_NVIC_DisableIRQ(UART_CONSOLE_DMAx_STREAMx_IRQn);
}

/**
 * @brief  Gets the current error state of the UART Console
 * @param  None
 * @retval 0 if no error occured
 *         positive value indicates error where each bit
 *         corresponds to a specific error defined in _UART_CONSOLE_ERROR
 * @note   -
 */
uint32_t uart_console_get_error(void)
{
    return uart_console_error;
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
    HAL_UART_Receive_IT(&h_uart_console, &uart_rx_buffer, 1);
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

/**
 * @brief  Newlib hook to allow printf/iprintf to appear on console
 *
 * @param  buf Pointer to data buffer containing the data to write
 * @param  len Length of data in bytes expected to be written
 *
 * @return Number of bytes written
 */
int __uart_console_write(char *buf, int len)
{
    uart_tx_data_t data = {
        .pbuf = NULL,
        .size = 0,
    };

    BaseType_t ret;
    const TickType_t ticks_to_wait = pdMS_TO_TICKS(2UL * dma_tx_max_time_ms);

    ret = xQueueReceive(h_uart_tx_avail_queue, &data.pbuf, ticks_to_wait);
    uart_console_error |= (pdPASS != ret) ? UART_CONSOLE_ERROR_TX_AVAIL_QUEUE_RECEIVE : 0UL;
    assert_param(0UL == uart_console_error);

    memcpy(data.pbuf, (uint8_t *)buf, len);
    data.size = len;

    ret = xQueueSend(h_uart_tx_ready_queue, &data, ticks_to_wait);
    uart_console_error |= (pdPASS != ret) ? UART_CONSOLE_ERROR_TX_READY_QUEUE_SEND : 0UL;
    assert_param(0UL == uart_console_error);

    return len;
}

