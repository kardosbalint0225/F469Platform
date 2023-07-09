/*
 * uart_console.c
 *
 *  Created on: 2023. jul. 9.
 *      Author: Balint
 */
#include "uart_console.h"
#include "stm32f4xx_hal.h"
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

typedef struct {
	uint8_t *pbuf;
	uint16_t size;
} uart_tx_data_t;

static SemaphoreHandle_t uart_tx_complete_semaphore_handle = NULL;
static StaticSemaphore_t uart_tx_complete_semaphore_storage;

#define UART_RX_QUEUE_LENGTH						8
static StaticQueue_t uart_rx_queue_struct;
static uint8_t		 uart_rx_queue_storage[UART_RX_QUEUE_LENGTH * sizeof(uint8_t)];
static QueueHandle_t uart_rx_queue_handle = NULL;

#define UART_TX_AVAILABLE_QUEUE_LENGTH				16
static StaticQueue_t uart_tx_available_struct;
static uint8_t       uart_tx_available_queue_storage[UART_TX_AVAILABLE_QUEUE_LENGTH * sizeof(uint8_t *)];
static QueueHandle_t uart_tx_available_queue_handle	= NULL;

#define UART_TX_READY_QUEUE_LENGTH					UART_TX_AVAILABLE_QUEUE_LENGTH
static StaticQueue_t uart_tx_ready_struct;
static uint8_t		 uart_tx_ready_queue_storage[UART_TX_READY_QUEUE_LENGTH * sizeof(uart_tx_data_t)];
static QueueHandle_t uart_tx_ready_queue_handle = NULL;

#define UART_WRITE_TASK_PRIORITY					2
#define UART_WRITE_TASK_STACKSIZE					(configMINIMAL_STACK_SIZE)
static StackType_t   uart_write_task_stack[UART_WRITE_TASK_STACKSIZE];
static StaticTask_t  uart_write_task_tcb;
static TaskHandle_t  uart_write_task_handle = NULL;

UART_HandleTypeDef huart3;
DMA_HandleTypeDef  hdma_usart3_tx;

static uint8_t uart_tx_buffer[configCOMMAND_INT_MAX_OUTPUT_SIZE*UART_TX_AVAILABLE_QUEUE_LENGTH];
static uint8_t uart_rx_buffer[4];

static void UART3_Init(void);
static void UART3_Deinit(void);
static void UART3_MspInit(UART_HandleTypeDef* huart);
static void UART3_MspDeInit(UART_HandleTypeDef* huart);
static void UART3_TxCpltCallback(UART_HandleTypeDef *huart);
static void UART3_RxCpltCallback(UART_HandleTypeDef *huart);

/**
  * @brief  UART writer gatekeeper task
  * @param  params not used
  * @retval None
  * @note	Task that performs the UART TX related jobs.
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

	for ( ;; )
	{
		ret = xQueueReceive(uart_tx_ready_queue_handle, &uart_tx_data, portMAX_DELAY);
		assert_param(pdPASS == ret);

		hal_status = HAL_UART_Transmit_DMA(&huart3, uart_tx_data.pbuf, uart_tx_data.size);
		assert_param(HAL_OK == hal_status);
		uart_tx_pending = uart_tx_data.pbuf;

		ret = xSemaphoreTake(uart_tx_complete_semaphore_handle, portMAX_DELAY);
		assert_param(pdPASS == ret);

		ret = xQueueSend(uart_tx_available_queue_handle, &uart_tx_pending, 0);
		assert_param(pdPASS == ret);
	}
}

/**
  * @brief  Initializes the UART Console
  * @param  None
  * @retval None
  * @note	This function initializes the UART2 peripheral,
  * 		creates the UART writer task, queues and semaphores
  */
void uart_console_init(void)
{
	UART3_Init();

	uart_tx_complete_semaphore_handle = xSemaphoreCreateBinaryStatic(&uart_tx_complete_semaphore_storage);
	assert_param(NULL != uart_tx_complete_semaphore_handle);
	xSemaphoreTake(uart_tx_complete_semaphore_handle, 0);

	uart_tx_available_queue_handle = xQueueCreateStatic(
									    UART_TX_AVAILABLE_QUEUE_LENGTH,
										sizeof(uint8_t *),
										uart_tx_available_queue_storage,
										&uart_tx_available_struct);
	assert_param(NULL != uart_tx_available_queue_handle);

	uart_tx_ready_queue_handle = xQueueCreateStatic(
								    UART_TX_READY_QUEUE_LENGTH,
									sizeof(uart_tx_data_t),
									uart_tx_ready_queue_storage,
									&uart_tx_ready_struct);
	assert_param(NULL != uart_tx_ready_queue_handle);

	uart_rx_queue_handle = xQueueCreateStatic(
						      UART_RX_QUEUE_LENGTH,
							  sizeof(uint8_t),
							  uart_rx_queue_storage,
							  &uart_rx_queue_struct);
	assert_param(NULL != uart_rx_queue_handle);

	for (uint8_t i = 0; i < UART_TX_AVAILABLE_QUEUE_LENGTH; i++) {
		uint8_t *buffer_address = &uart_tx_buffer[i*configCOMMAND_INT_MAX_OUTPUT_SIZE];
		xQueueSend(uart_tx_available_queue_handle, &buffer_address, 0);
	}

	uart_write_task_handle = xTaskCreateStatic(
							    uart_console_write_task,
								"UART write",
								UART_WRITE_TASK_STACKSIZE,
								NULL,
								UART_WRITE_TASK_PRIORITY,
								uart_write_task_stack,
								&uart_write_task_tcb);
	assert_param(NULL != uart_write_task_handle);

	HAL_StatusTypeDef hal_status = HAL_UART_Receive_IT(&huart3, &uart_rx_buffer[0], 1);
	assert_param(HAL_OK == hal_status);
}

/**
  * @brief  Deinitializes the UART Console
  * @param  None
  * @retval None
  * @note	This function deinitializes the UART3peripheral
  * 		and deletes the UART writer task, queues and semaphores
  */
void uart_console_deinit(void)
{
	UART3_Deinit();

	vTaskDelete(uart_write_task_handle);
	vQueueDelete(uart_tx_available_queue_handle);
	vQueueDelete(uart_tx_ready_queue_handle);
	vQueueDelete(uart_rx_queue_handle);
	vSemaphoreDelete(uart_tx_complete_semaphore_handle);

	uart_write_task_handle            = NULL;
	uart_tx_available_queue_handle    = NULL;
	uart_tx_ready_queue_handle        = NULL;
	uart_rx_queue_handle              = NULL;
	uart_tx_complete_semaphore_handle = NULL;
}

/**
  * @brief  Initializes the UART3 peripheral
  * @param  None
  * @retval None
  * @note	The communication is configured 115200 Baudrate 8N1 with no
  * 		flowcontrol.
  */
static void UART3_Init(void)
{
	huart3.Instance          = USART3;
	huart3.Init.BaudRate     = 115200;
	huart3.Init.WordLength   = UART_WORDLENGTH_8B;
	huart3.Init.StopBits     = UART_STOPBITS_1;
	huart3.Init.Parity       = UART_PARITY_NONE;
	huart3.Init.Mode         = UART_MODE_TX_RX;
	huart3.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
	huart3.Init.OverSampling = UART_OVERSAMPLING_16;

	HAL_StatusTypeDef ret;

	ret = HAL_UART_RegisterCallback(&huart3, HAL_UART_MSPINIT_CB_ID, UART3_MspInit);
	assert_param(HAL_OK == ret);

	ret = HAL_UART_RegisterCallback(&huart3, HAL_UART_MSPDEINIT_CB_ID, UART3_MspDeInit);
	assert_param(HAL_OK == ret);

	ret = HAL_UART_Init(&huart3);
	assert_param(HAL_OK == ret);

	ret = HAL_UART_RegisterCallback(&huart3, HAL_UART_TX_COMPLETE_CB_ID, UART3_TxCpltCallback);
	assert_param(HAL_OK == ret);

	ret = HAL_UART_RegisterCallback(&huart3, HAL_UART_RX_COMPLETE_CB_ID, UART3_RxCpltCallback);
	assert_param(HAL_OK == ret);
}

/**
  * @brief  Initializes the UART3 peripheral low-level
  * @param  uartHandle
  * @retval None
  * @note	This function is called by the HAL library
  * @note	DMA1 Stream3 is used for TX. UART TX and DMA priority
  * 		are set to 14 but possibly can be set to 15
  */
static void UART3_MspInit(UART_HandleTypeDef* huart)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	__HAL_RCC_USART3_CLK_ENABLE();
	__HAL_RCC_DMA1_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	GPIO_InitStruct.Pin       = GPIO_PIN_10 | GPIO_PIN_11;
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_NOPULL;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	hdma_usart3_tx.Instance                 = DMA1_Stream3;
	hdma_usart3_tx.Init.Channel             = DMA_CHANNEL_4;
	hdma_usart3_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
	hdma_usart3_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
	hdma_usart3_tx.Init.MemInc              = DMA_MINC_ENABLE;
	hdma_usart3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_usart3_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
	hdma_usart3_tx.Init.Mode                = DMA_NORMAL;
	hdma_usart3_tx.Init.Priority            = DMA_PRIORITY_LOW;
	hdma_usart3_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;

	HAL_StatusTypeDef ret = HAL_DMA_Init(&hdma_usart3_tx);
	assert_param(HAL_OK == ret);

	__HAL_LINKDMA(huart, hdmatx, hdma_usart3_tx);

	HAL_NVIC_SetPriority(USART3_IRQn, 14, 0);
	HAL_NVIC_EnableIRQ(USART3_IRQn);

	HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 14, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);
}

/**
  * @brief  Deinitializes the UART3 peripheral
  * @param  None
  * @retval None
  * @note	-
  *
  */
static void UART3_Deinit(void)
{
	HAL_StatusTypeDef ret;

	ret = HAL_UART_UnRegisterCallback(&huart3, HAL_UART_TX_COMPLETE_CB_ID);
	assert_param(HAL_OK == ret);

	ret = HAL_UART_DeInit(&huart3);
	assert_param(HAL_OK == ret);

	ret = HAL_UART_UnRegisterCallback(&huart3, HAL_UART_MSPINIT_CB_ID);
	assert_param(HAL_OK == ret);

	ret = HAL_UART_UnRegisterCallback(&huart3, HAL_UART_MSPDEINIT_CB_ID);
	assert_param(HAL_OK == ret);
}

/**
  * @brief  Deinitializes the UART3 peripheral low-level
  * @param  uartHandle
  * @retval None
  * @note	This function is called by the HAL library
  * @note 	It wont disable the DMA1 peripheral clock since the
  * 		DMA1 might be used by other peripherals
  * @note	It wont disable the GPIOB peripheral clock since the
  * 		GPIOB might be used by other peripherals
  */
static void UART3_MspDeInit(UART_HandleTypeDef* huart)
{
	__HAL_RCC_USART3_CLK_DISABLE();

	HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10 | GPIO_PIN_11);

	HAL_DMA_DeInit(huart->hdmatx);

	HAL_NVIC_DisableIRQ(USART3_IRQn);
	HAL_NVIC_DisableIRQ(DMA1_Stream3_IRQn);
}

/**
  * @brief  UART3 Transfer complete callback
  * @param  huart
  * @retval None
  * @note	This function is called by the HAL library
  * 		when the DMA1 is finished transferring data
  */
static void UART3_TxCpltCallback(UART_HandleTypeDef *huart)
{
	portBASE_TYPE higher_priority_task_woken = pdFALSE;
	xSemaphoreGiveFromISR(uart_tx_complete_semaphore_handle, &higher_priority_task_woken);
	portYIELD_FROM_ISR(higher_priority_task_woken);
}

/**
  * @brief  UART3 Receive complete callback
  * @param  huart
  * @retval None
  * @note	This function is called by the HAL library
  * 		when a character is arrived.
  */
static void UART3_RxCpltCallback(UART_HandleTypeDef *huart)
{
	portBASE_TYPE higher_priority_task_woken = pdFALSE;
	xQueueSendFromISR(uart_rx_queue_handle, &uart_rx_buffer[0], &higher_priority_task_woken);
	HAL_UART_Receive_IT(&huart3, &uart_rx_buffer[0], 1);
	portYIELD_FROM_ISR(higher_priority_task_woken);
}

/**
 * @brief Newlib hook to allow printf/iprintf to appear on console
 *
 * @param buf Pointer to data buffer containing the data to write
 * @param len Length of data in bytes expected to be written
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
	const TickType_t ticks_to_wait = pdMS_TO_TICKS(200);

	ret = xQueueReceive(uart_tx_available_queue_handle, &data.pbuf, ticks_to_wait);
	assert_param(pdTRUE == ret);

	memcpy(data.pbuf, (uint8_t *)buf, len);
	data.size = len;

	ret = xQueueSend(uart_tx_ready_queue_handle, &data, ticks_to_wait);
	assert_param(pdTRUE == ret);

	return len;
}

