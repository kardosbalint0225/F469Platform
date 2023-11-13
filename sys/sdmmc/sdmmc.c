/*
 * sdmmc.c
 *
 *  Created on: 2023. okt. 25.
 *      Author: Balint
 */
#include "sdmmc.h"
#include "sdmmc_config.h"
#include "stm32f4xx_hal.h"

#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

SD_HandleTypeDef h_sdio;
DMA_HandleTypeDef h_sdio_dma_tx;
DMA_HandleTypeDef h_sdio_dma_rx;

static SemaphoreHandle_t h_sdio_tx_cplt_semphr = NULL;
static StaticSemaphore_t sdio_tx_cplt_semphr_storage;
static SemaphoreHandle_t h_sdio_rx_cplt_semphr = NULL;
static StaticSemaphore_t sdio_rx_cplt_semphr_storage;

static int sdio_init(void);
static int sdio_deinit(void);
static void sdio_msp_init(SD_HandleTypeDef *h_sd);
static void sdio_msp_deinit(SD_HandleTypeDef *h_sd);
static void sdio_tx_cplt_callback(SD_HandleTypeDef *h_sd);
static void sdio_rx_cplt_callback(SD_HandleTypeDef *h_sd);
static bool is_word_aligned(const void *pbuf);

int sdmmc_init(void)
{
    h_sdio_tx_cplt_semphr = xSemaphoreCreateBinaryStatic(&sdio_tx_cplt_semphr_storage);
    assert_param(NULL != h_sdio_tx_cplt_semphr);
    h_sdio_rx_cplt_semphr = xSemaphoreCreateBinaryStatic(&sdio_rx_cplt_semphr_storage);
    assert_param(NULL != h_sdio_rx_cplt_semphr);

    sdio_init();

    return 0;
}

int sdmmc_deinit(void)
{
    vSemaphoreDelete(h_sdio_tx_cplt_semphr);
    vSemaphoreDelete(h_sdio_rx_cplt_semphr);

    h_sdio_tx_cplt_semphr = NULL;
    h_sdio_rx_cplt_semphr = NULL;

    sdio_deinit();

    return 0;
}

int sdmmc_read_blocks(uint32_t block_addr,
                      uint16_t block_size,
                      uint16_t block_num,
                      void *data,
                      uint16_t *done)
{
    assert_param(NULL != data);

    if (true == is_word_aligned(data))
    {
        HAL_StatusTypeDef hal_status = HAL_SD_ReadBlocks_DMA(&h_sdio, (uint8_t *)data, block_addr, block_num);
        assert_param(HAL_OK == hal_status); //TODO: return value

        const TickType_t ticks_to_wait = portMAX_DELAY;
        BaseType_t ret = xSemaphoreTake(h_sdio_rx_cplt_semphr, ticks_to_wait);
        assert_param(pdPASS == ret); //TODO: return value
    }
    else
    {
        uint8_t *work_area = pvPortMalloc(SDMMC_SDHC_BLOCK_SIZE);
        assert_param(NULL != work_area); //TODO: return value

        for (uint32_t block = 0; block < (uint32_t)block_num; block++)
        {
            uint32_t address = block_addr + block;
            HAL_StatusTypeDef hal_status = HAL_SD_ReadBlocks_DMA(&h_sdio, work_area, address, 1);
            assert_param(HAL_OK == hal_status); //TODO: return value

            const TickType_t ticks_to_wait = portMAX_DELAY;
            BaseType_t ret = xSemaphoreTake(h_sdio_rx_cplt_semphr, ticks_to_wait);
            assert_param(pdPASS == ret); //TODO: return value

            memcpy(data + SDMMC_SDHC_BLOCK_SIZE * block, work_area, SDMMC_SDHC_BLOCK_SIZE);
        }

        vPortFree(work_area);
    }

    return 0;
}

int sdmmc_write_blocks(uint32_t block_addr,
                       uint16_t block_size,
                       uint16_t block_num,
                       const void *data,
                       uint16_t *done)
{
    assert_param(NULL != data);

    if (true == is_word_aligned(data))
    {
        HAL_StatusTypeDef hal_status = HAL_SD_WriteBlocks_DMA(&h_sdio, (uint8_t *)data, block_addr, block_num);
        assert_param(HAL_OK == hal_status); //TODO: return value

        const TickType_t ticks_to_wait = portMAX_DELAY;
        BaseType_t ret = xSemaphoreTake(h_sdio_tx_cplt_semphr, ticks_to_wait);
        assert_param(pdPASS == ret); //TODO: return value
    }
    else
    {
        uint8_t *work_area = pvPortMalloc(SDMMC_SDHC_BLOCK_SIZE);
        assert_param(NULL != work_area); //TODO: return value

        for (uint32_t block = 0; block < (uint32_t)block_num; block++)
        {
            memcpy(work_area, data + SDMMC_SDHC_BLOCK_SIZE * block, SDMMC_SDHC_BLOCK_SIZE);
            uint32_t address = block_addr + block;
            HAL_StatusTypeDef hal_status = HAL_SD_WriteBlocks_DMA(&h_sdio, work_area, address, 1);
            assert_param(HAL_OK == hal_status); //TODO: return value

            const TickType_t ticks_to_wait = portMAX_DELAY;
            BaseType_t ret = xSemaphoreTake(h_sdio_tx_cplt_semphr, ticks_to_wait);
            assert_param(pdPASS == ret); //TODO: return value
        }

        vPortFree(work_area);
    }

    return 0;
}

int sdmmc_erase_blocks(uint32_t block_addr, uint16_t block_num)
{
    uint8_t *work_area = pvPortMalloc(SDMMC_SDHC_BLOCK_SIZE);

    memset(work_area, 0, SDMMC_SDHC_BLOCK_SIZE);
    sdmmc_write_blocks(block_addr, SDMMC_SDHC_BLOCK_SIZE, block_num, work_area, NULL); //TODO: return value

    vPortFree(work_area);

    return 0;
}

uint64_t sdmmc_get_capacity(void)
{
    HAL_SD_CardInfoTypeDef ci = {0};
    HAL_StatusTypeDef ret = HAL_SD_GetCardInfo(&h_sdio, &ci);
    assert_param(HAL_OK == ret);

    uint64_t capacity = ((uint64_t)ci.BlockNbr) * ((uint64_t)ci.BlockSize);
    return capacity;
}

static bool is_word_aligned(const void *pbuf)
{
    assert_param(NULL != pbuf);
    return (0 == (((size_t)pbuf) & (sizeof(size_t) - 1))) ? true : false;
}

static int sdio_init(void)
{
    h_sdio.Instance = SDIO;
    h_sdio.Init.ClockDiv = SDIO_TRANSFER_CLK_DIV;
    h_sdio.Init.BusWide = SDIO_BUS_WIDE_1B;
    h_sdio.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
    h_sdio.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
    h_sdio.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
    h_sdio.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_ENABLE;

    HAL_StatusTypeDef ret;

    ret = HAL_SD_RegisterCallback(&h_sdio, HAL_SD_MSP_INIT_CB_ID, sdio_msp_init);
    assert_param(HAL_OK == ret);

    ret = HAL_SD_RegisterCallback(&h_sdio, HAL_SD_MSP_DEINIT_CB_ID, sdio_msp_deinit);
    assert_param(HAL_OK == ret);

    ret = HAL_SD_Init(&h_sdio);
    assert_param(HAL_OK == ret);

    ret = HAL_SD_RegisterCallback(&h_sdio, HAL_SD_TX_CPLT_CB_ID, sdio_tx_cplt_callback);
    assert_param(HAL_OK == ret);

    ret = HAL_SD_RegisterCallback(&h_sdio, HAL_SD_RX_CPLT_CB_ID, sdio_rx_cplt_callback);
    assert_param(HAL_OK == ret);

    ret = HAL_SD_ConfigWideBusOperation(&h_sdio, SDIO_BUS_WIDE_4B);
    assert_param(HAL_OK == ret);

    //TODO: Error callback

    return 0;
}

static void sdio_msp_init(SD_HandleTypeDef *h_sd)
{
    SDMMC_CMD_PIN_GPIO_CLK_ENABLE();
    SDMMC_CLK_PIN_GPIO_CLK_ENABLE();
    SDMMC_D3_PIN_GPIO_CLK_ENABLE();
    SDMMC_D2_PIN_GPIO_CLK_ENABLE();
    SDMMC_D1_PIN_GPIO_CLK_ENABLE();
    SDMMC_D0_PIN_GPIO_CLK_ENABLE();

    SDMMC_DMAx_CLK_ENABLE();
    __HAL_RCC_SDIO_CLK_ENABLE();

    GPIO_InitTypeDef cmd_pin = {
        .Pin = SDMMC_CMD_PIN,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_PULLUP,
        .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
        .Alternate = SDMMC_GPIO_AFx_SDIO,
    };
    HAL_GPIO_Init(SDMMC_CMD_PIN_GPIO_PORT, &cmd_pin);

    GPIO_InitTypeDef clk_pin = {
        .Pin = SDMMC_CLK_PIN,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_PULLUP,
        .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
        .Alternate = SDMMC_GPIO_AFx_SDIO,
    };
    HAL_GPIO_Init(SDMMC_CLK_PIN_GPIO_PORT, &clk_pin);

    GPIO_InitTypeDef d3_pin = {
       .Pin = SDMMC_D3_PIN,
       .Mode = GPIO_MODE_AF_PP,
       .Pull = GPIO_PULLUP,
       .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
       .Alternate = SDMMC_GPIO_AFx_SDIO,
    };
    HAL_GPIO_Init(SDMMC_D3_PIN_GPIO_PORT, &d3_pin);

    GPIO_InitTypeDef d2_pin = {
       .Pin = SDMMC_D2_PIN,
       .Mode = GPIO_MODE_AF_PP,
       .Pull = GPIO_PULLUP,
       .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
       .Alternate = SDMMC_GPIO_AFx_SDIO,
    };
    HAL_GPIO_Init(SDMMC_D2_PIN_GPIO_PORT, &d2_pin);

    GPIO_InitTypeDef d1_pin = {
       .Pin = SDMMC_D1_PIN,
       .Mode = GPIO_MODE_AF_PP,
       .Pull = GPIO_PULLUP,
       .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
       .Alternate = SDMMC_GPIO_AFx_SDIO,
    };
    HAL_GPIO_Init(SDMMC_D1_PIN_GPIO_PORT, &d1_pin);

    GPIO_InitTypeDef d0_pin = {
       .Pin = SDMMC_D0_PIN,
       .Mode = GPIO_MODE_AF_PP,
       .Pull = GPIO_PULLUP,
       .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
       .Alternate = SDMMC_GPIO_AFx_SDIO,
    };
    HAL_GPIO_Init(SDMMC_D0_PIN_GPIO_PORT, &d0_pin);

    HAL_NVIC_SetPriority(SDIO_IRQn, SDMMC_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(SDIO_IRQn);

    HAL_StatusTypeDef ret;

    h_sdio_dma_rx.Instance = SDMMC_DMAx_RX_STREAM;
    h_sdio_dma_rx.Init.Channel = SDMMC_DMAx_RX_CHANNEL;
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

    __HAL_LINKDMA(h_sd, hdmarx, h_sdio_dma_rx);

    ret = HAL_DMA_DeInit(&h_sdio_dma_rx);
    assert_param(HAL_OK == ret);
    ret = HAL_DMA_Init(&h_sdio_dma_rx);
    assert_param(HAL_OK == ret);

    HAL_NVIC_SetPriority(SDMMC_DMAx_RX_IRQn, SDMMC_DMAx_RX_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(SDMMC_DMAx_RX_IRQn);

    h_sdio_dma_tx.Instance = SDMMC_DMAx_TX_STREAM;
    h_sdio_dma_tx.Init.Channel = SDMMC_DMAx_TX_CHANNEL;
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

    __HAL_LINKDMA(h_sd, hdmatx, h_sdio_dma_tx);

    ret = HAL_DMA_DeInit(&h_sdio_dma_tx);
    assert_param(HAL_OK == ret);
    ret = HAL_DMA_Init(&h_sdio_dma_tx);
    assert_param(HAL_OK == ret);

    HAL_NVIC_SetPriority(SDMMC_DMAx_TX_IRQn, SDMMC_DMAx_TX_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(SDMMC_DMAx_TX_IRQn);
}

static int sdio_deinit(void)
{
    HAL_StatusTypeDef ret;
    ret = HAL_SD_DeInit(&h_sdio);
    assert_param(HAL_OK == ret);
    return 0;
}

static void sdio_msp_deinit(SD_HandleTypeDef *h_sd)
{
    HAL_StatusTypeDef ret;

    HAL_NVIC_DisableIRQ(SDMMC_DMAx_RX_IRQn);
    HAL_NVIC_DisableIRQ(SDMMC_DMAx_TX_IRQn);

    ret = HAL_DMA_DeInit(&h_sdio_dma_rx);
    assert_param(HAL_OK == ret);
    ret = HAL_DMA_DeInit(&h_sdio_dma_tx);
    assert_param(HAL_OK == ret);

    HAL_NVIC_DisableIRQ(SDIO_IRQn);

    __HAL_RCC_SDIO_CLK_DISABLE();
}

static void sdio_tx_cplt_callback(SD_HandleTypeDef *h_sd)
{
    BaseType_t higher_priority_task_woken = pdFALSE;
    xSemaphoreGiveFromISR(h_sdio_tx_cplt_semphr, &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

static void sdio_rx_cplt_callback(SD_HandleTypeDef *h_sd)
{
    BaseType_t higher_priority_task_woken = pdFALSE;
    xSemaphoreGiveFromISR(h_sdio_rx_cplt_semphr, &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
}





