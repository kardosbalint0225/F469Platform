/**
 * MIT License
 *
 * Copyright (c) 2024 Balint Kardos
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/**
 * @ingroup     system_dma
 *
 * @file        dma.h
 * @brief       DMA management for the peripherals used by the application
 */
#ifndef __DMA_H__
#define __DMA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/**
 * @brief  Initializes DMA for the STDIO UART transmission.
 *
 * This function configures the DMA controller to transfer data from memory to the UART peripheral
 * for transmission.
 *
 * @param  huart Pointer to a UART_HandleTypeDef structure that contains
 *               the configuration information for the specified UART module.
 *
 * @return HAL status
 */
HAL_StatusTypeDef stdio_uart_dma_init(UART_HandleTypeDef *huart);

/**
 * @brief  De-initializes DMA for the STDIO UART transmission.
 *
 * @param  huart Pointer to a UART_HandleTypeDef structure that contains
 *               the configuration information for the specified UART module.
 *
 * @return HAL status
 */
HAL_StatusTypeDef stdio_uart_dma_deinit(UART_HandleTypeDef *huart);

/**
 * @brief  Initializes DMA for SDIO receive operation.
 *
 * This function initializes the DMA controller for SDIO receive operation.
 *
 * @param  h_sd Pointer to the SDIO handle structure.
 *
 * @return HAL status
 */
HAL_StatusTypeDef sdcard_sdio_dma_rx_init(SD_HandleTypeDef *h_sd);

/**
 * @brief  Initializes DMA for SDIO transmit operation.
 *
 * This function initializes the DMA controller for SDIO transmit operation.
 *
 * @param  h_sd Pointer to the SDIO handle structure.
 *
 * @return HAL status
 */
HAL_StatusTypeDef sdcard_sdio_dma_tx_init(SD_HandleTypeDef *h_sd);

/**
 * @brief  De-initializes DMA for SDIO receive operation.
 *
 * This function de-initializes the DMA controller for SDIO receive operation.
 *
 * @param  h_sd Pointer to the SDIO handle structure.
 *
 * @return HAL status
 */
HAL_StatusTypeDef sdcard_sdio_dma_rx_deinit(SD_HandleTypeDef *h_sd);

/**
 * @brief  De-initializes DMA for SDIO transmit operation.
 *
 * This function de-initializes the DMA controller for SDIO transmit operation.
 *
 * @param  h_sd Pointer to the SDIO handle structure.
 *
 * @return HAL status
 */
HAL_StatusTypeDef sdcard_sdio_dma_tx_deinit(SD_HandleTypeDef *h_sd);

#ifdef __cplusplus
}
#endif
#endif /* __DMA_H__ */
