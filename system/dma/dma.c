/*
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
 * @{
 * @file        dma.c
 * @brief       DMA management for the peripherals used by the application
 */
#include "dma.h"
#include "rcc.h"
#include "stm32f4xx_hal.h"

#include "stdio_uart_config.h"
#include "sdcard_config.h"

static DMA_HandleTypeDef h_stdio_uart_dma_tx;
static DMA_HandleTypeDef h_sdio_dma_tx;
static DMA_HandleTypeDef h_sdio_dma_rx;

HAL_StatusTypeDef stdio_uart_dma_init(UART_HandleTypeDef *huart)
{
    HAL_StatusTypeDef ret;

    rcc_periph_clk_enable((const void *)STDIO_UART_DMAx);

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

    ret = HAL_DMA_Init(&h_stdio_uart_dma_tx);
    if (HAL_OK != ret)
    {
        return ret;
    }

    __HAL_LINKDMA(huart, hdmatx, h_stdio_uart_dma_tx);

    HAL_NVIC_SetPriority(STDIO_UART_DMAx_STREAMx_IRQn, STDIO_UART_DMAx_STREAMx_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(STDIO_UART_DMAx_STREAMx_IRQn);

    return HAL_OK;
}

HAL_StatusTypeDef stdio_uart_dma_deinit(UART_HandleTypeDef *huart)
{
    HAL_StatusTypeDef ret;

    ret = HAL_DMA_DeInit(huart->hdmatx);
    if (HAL_OK != ret)
    {
        return ret;
    }

    HAL_NVIC_DisableIRQ(STDIO_UART_DMAx_STREAMx_IRQn);
    rcc_periph_clk_disable((const void *)STDIO_UART_DMAx);

    return HAL_OK;
}

HAL_StatusTypeDef sdcard_sdio_dma_rx_init(SD_HandleTypeDef *h_sd)
{
    HAL_StatusTypeDef ret;

    rcc_periph_clk_enable((const void *)SDCARD_DMAx);

    h_sdio_dma_rx.Instance = SDCARD_DMAx_RX_STREAM;
    h_sdio_dma_rx.Init.Channel = SDCARD_DMAx_RX_CHANNEL;
    h_sdio_dma_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    h_sdio_dma_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    h_sdio_dma_rx.Init.MemInc = DMA_MINC_ENABLE;
    h_sdio_dma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    h_sdio_dma_rx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    h_sdio_dma_rx.Init.Mode = DMA_PFCTRL;
    h_sdio_dma_rx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    h_sdio_dma_rx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    h_sdio_dma_rx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    h_sdio_dma_rx.Init.MemBurst = DMA_MBURST_INC4;
    h_sdio_dma_rx.Init.PeriphBurst = DMA_PBURST_INC4;

    ret = HAL_DMA_Init(&h_sdio_dma_rx);
    if (HAL_OK != ret)
    {
        return ret;
    }

    __HAL_LINKDMA(h_sd, hdmarx, h_sdio_dma_rx);


    HAL_NVIC_SetPriority(SDCARD_DMAx_RX_IRQn, SDCARD_DMAx_RX_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(SDCARD_DMAx_RX_IRQn);

    return HAL_OK;
}

HAL_StatusTypeDef sdcard_sdio_dma_tx_init(SD_HandleTypeDef *h_sd)
{
    HAL_StatusTypeDef ret;

    rcc_periph_clk_enable((const void *)SDCARD_DMAx);

    h_sdio_dma_tx.Instance = SDCARD_DMAx_TX_STREAM;
    h_sdio_dma_tx.Init.Channel = SDCARD_DMAx_TX_CHANNEL;
    h_sdio_dma_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    h_sdio_dma_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    h_sdio_dma_tx.Init.MemInc = DMA_MINC_ENABLE;
    h_sdio_dma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    h_sdio_dma_tx.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    h_sdio_dma_tx.Init.Mode = DMA_PFCTRL;
    h_sdio_dma_tx.Init.Priority = DMA_PRIORITY_VERY_HIGH;
    h_sdio_dma_tx.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
    h_sdio_dma_tx.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    h_sdio_dma_tx.Init.MemBurst = DMA_MBURST_INC4;
    h_sdio_dma_tx.Init.PeriphBurst = DMA_PBURST_INC4;

    ret = HAL_DMA_Init(&h_sdio_dma_tx);
    if (HAL_OK != ret)
    {
        return ret;
    }

    __HAL_LINKDMA(h_sd, hdmatx, h_sdio_dma_tx);

    HAL_NVIC_SetPriority(SDCARD_DMAx_TX_IRQn, SDCARD_DMAx_TX_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(SDCARD_DMAx_TX_IRQn);

    return HAL_OK;
}

HAL_StatusTypeDef sdcard_sdio_dma_rx_deinit(SD_HandleTypeDef *h_sd)
{
    HAL_StatusTypeDef ret;

    ret = HAL_DMA_DeInit(&h_sdio_dma_rx);
    if (HAL_OK != ret)
    {
        return ret;
    }

    rcc_periph_clk_disable((const void *)SDCARD_DMAx);

    HAL_NVIC_DisableIRQ(SDCARD_DMAx_RX_IRQn);

    return HAL_OK;
}

HAL_StatusTypeDef sdcard_sdio_dma_tx_deinit(SD_HandleTypeDef *h_sd)
{
    HAL_StatusTypeDef ret;

    ret = HAL_DMA_DeInit(&h_sdio_dma_tx);
    if (HAL_OK != ret)
    {
        return ret;
    }

    rcc_periph_clk_disable((const void *)SDCARD_DMAx);

    HAL_NVIC_DisableIRQ(SDCARD_DMAx_TX_IRQn);

    return HAL_OK;
}

/**
 * @brief STDIO UART DMA Stream Interrupt Handler
 */
void STDIO_UART_DMA_STREAM_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&h_stdio_uart_dma_tx);
}

/**
 * @brief SDCARD DMA Rx Stream Interrupt Handler
 */
void SDCARD_DMAx_RX_STREAM_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&h_sdio_dma_rx);
}

/**
 * @brief SDCARD DMA Tx Stream Interrupt Handler
 */
void SDCARD_DMAx_TX_STREAM_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&h_sdio_dma_tx);
}
/** @} */

