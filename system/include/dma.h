/*
 * dma.h
 *
 *  Created on: 2023. nov. 25.
 *      Author: Balint
 */

#ifndef __DMA_H__
#define __DMA_H__

#include "stm32f4xx_hal.h"

HAL_StatusTypeDef stdio_uart_dma_init(UART_HandleTypeDef *huart);
HAL_StatusTypeDef stdio_uart_dma_deinit(UART_HandleTypeDef *huart);

HAL_StatusTypeDef sdcard_sdio_dma_rx_init(SD_HandleTypeDef *h_sd);
HAL_StatusTypeDef sdcard_sdio_dma_tx_init(SD_HandleTypeDef *h_sd);
HAL_StatusTypeDef sdcard_sdio_dma_rx_deinit(SD_HandleTypeDef *h_sd);
HAL_StatusTypeDef sdcard_sdio_dma_tx_deinit(SD_HandleTypeDef *h_sd);

#endif /* __DMA_H__ */
