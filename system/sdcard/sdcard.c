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
 * @ingroup     system_sdcard_driver
 * @{
 * @file        sdcard.c
 * @brief       Driver for reading and writing SD Cards using the SDIO.
 */
#include "sdcard.h"
#include "sdcard_config.h"
#include "stm32f4xx_hal.h"
#include "hal_errno.h"

#include "rcc.h"
#include "dma.h"
#include "gpio.h"

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

static SD_HandleTypeDef h_sdio;

static SemaphoreHandle_t _tx_cplt_semphr = NULL;
static StaticSemaphore_t _tx_cplt_semphr_storage;
static SemaphoreHandle_t _rx_cplt_semphr = NULL;
static StaticSemaphore_t _rx_cplt_semphr_storage;

static HAL_StatusTypeDef _error = HAL_OK;

static int sdio_init(void);
static int sdio_deinit(void);
static void sdio_msp_init(SD_HandleTypeDef *h_sd);
static void sdio_msp_deinit(SD_HandleTypeDef *h_sd);
static void sdio_tx_cplt_callback(SD_HandleTypeDef *h_sd);
static void sdio_rx_cplt_callback(SD_HandleTypeDef *h_sd);
static void sdio_error_callback(SD_HandleTypeDef *h_sd);
static void error_handler(void);
static bool is_word_aligned(const void *pbuf);
static int sd_error_to_errno(const uint32_t error);

int sdcard_init(void)
{
    _tx_cplt_semphr = xSemaphoreCreateBinaryStatic(&_tx_cplt_semphr_storage);
    assert(_tx_cplt_semphr);
    _rx_cplt_semphr = xSemaphoreCreateBinaryStatic(&_rx_cplt_semphr_storage);
    assert(_rx_cplt_semphr);

    return sdio_init();
}

int sdcard_deinit(void)
{
    vSemaphoreDelete(_tx_cplt_semphr);
    vSemaphoreDelete(_rx_cplt_semphr);

    _tx_cplt_semphr = NULL;
    _rx_cplt_semphr = NULL;

    return sdio_deinit();
}

int sdcard_read_blocks(uint32_t block_addr, uint16_t block_num, void *data)
{
    assert(data);
    assert(block_num);

    HAL_StatusTypeDef hal_status;
    TimeOut_t timeout;
    TickType_t ticks_to_wait = pdMS_TO_TICKS(2 * SDCARD_DMA_BLOCK_TRANSFER_TIMEOUT_MS);
    vTaskSetTimeOutState(&timeout);
    BaseType_t timedout = xTaskCheckForTimeOut(&timeout, &ticks_to_wait);

    while ((HAL_SD_CARD_TRANSFER != HAL_SD_GetCardState(&h_sdio)) && (pdFALSE == timedout))
    {
        vTaskDelay(pdMS_TO_TICKS(1));
        timedout = xTaskCheckForTimeOut(&timeout, &ticks_to_wait);
    }

    if (pdTRUE == timedout)
    {
        return -ETIMEDOUT;
    }

    if (true == is_word_aligned(data))
    {
        hal_status = HAL_SD_ReadBlocks_DMA(&h_sdio, (uint8_t *)data, block_addr, (uint32_t)block_num);
        if (HAL_OK != hal_status)
        {
            return sd_error_to_errno(h_sdio.ErrorCode);
        }

        const TickType_t ticks_to_wait = pdMS_TO_TICKS(SDCARD_DMA_BLOCK_TRANSFER_TIMEOUT_MS * block_num);
        BaseType_t ret = xSemaphoreTake(_rx_cplt_semphr, ticks_to_wait);
        if (pdTRUE != ret)
        {
            return -ETIMEDOUT;
        }
    }
    else
    {
        uint8_t *work_area = pvPortMalloc(SDCARD_SDHC_BLOCK_SIZE);
        if (NULL == work_area)
        {
            return -ENOMEM;
        }

        for (uint32_t block = 0; block < (uint32_t)block_num; block++)
        {
            uint32_t address = block_addr + block;
            hal_status = HAL_SD_ReadBlocks_DMA(&h_sdio, work_area, address, 1);
            if (HAL_OK != hal_status)
            {
                vPortFree(work_area);
                return sd_error_to_errno(h_sdio.ErrorCode);
            }

            const TickType_t ticks_to_wait = pdMS_TO_TICKS(SDCARD_DMA_BLOCK_TRANSFER_TIMEOUT_MS);
            BaseType_t ret = xSemaphoreTake(_rx_cplt_semphr, ticks_to_wait);
            if (pdTRUE != ret)
            {
                return -ETIMEDOUT;
            }

            memcpy(data + SDCARD_SDHC_BLOCK_SIZE * block, work_area, SDCARD_SDHC_BLOCK_SIZE);
        }

        vPortFree(work_area);
    }

    return 0;
}

int sdcard_write_blocks(uint32_t block_addr, uint16_t block_num, const void *data)
{
    assert(data);
    assert(block_num);

    HAL_StatusTypeDef hal_status;
    TimeOut_t timeout;
    TickType_t ticks_to_wait = pdMS_TO_TICKS(2 * SDCARD_DMA_BLOCK_TRANSFER_TIMEOUT_MS);
    vTaskSetTimeOutState(&timeout);
    BaseType_t timedout = xTaskCheckForTimeOut(&timeout, &ticks_to_wait);

    while ((HAL_SD_CARD_TRANSFER != HAL_SD_GetCardState(&h_sdio)) && (pdFALSE == timedout))
    {
        vTaskDelay(pdMS_TO_TICKS(1));
        timedout = xTaskCheckForTimeOut(&timeout, &ticks_to_wait);
    }

    if (pdTRUE == timedout)
    {
        return -ETIMEDOUT;
    }

    if (true == is_word_aligned(data))
    {
        hal_status = HAL_SD_WriteBlocks_DMA(&h_sdio, (uint8_t *)data, block_addr, (uint32_t)block_num);
        if (HAL_OK != hal_status)
        {
            return sd_error_to_errno(h_sdio.ErrorCode);
        }

        const TickType_t ticks_to_wait = pdMS_TO_TICKS(2 * SDCARD_DMA_BLOCK_TRANSFER_TIMEOUT_MS * block_num);
        BaseType_t ret = xSemaphoreTake(_tx_cplt_semphr, ticks_to_wait);
        if (pdTRUE != ret)
        {
            return -ETIMEDOUT;
        }
    }
    else
    {
        uint8_t *work_area = pvPortMalloc(SDCARD_SDHC_BLOCK_SIZE);
        if (NULL == work_area)
        {
            return -ENOMEM;
        }

        for (uint32_t block = 0; block < (uint32_t)block_num; block++)
        {
            memcpy(work_area, data + SDCARD_SDHC_BLOCK_SIZE * block, SDCARD_SDHC_BLOCK_SIZE);
            uint32_t address = block_addr + block;
            hal_status = HAL_SD_WriteBlocks_DMA(&h_sdio, work_area, address, 1);
            if (HAL_OK != hal_status)
            {
                vPortFree(work_area);
                return sd_error_to_errno(h_sdio.ErrorCode);
            }

            const TickType_t ticks_to_wait = pdMS_TO_TICKS(2 * SDCARD_DMA_BLOCK_TRANSFER_TIMEOUT_MS);
            BaseType_t ret = xSemaphoreTake(_tx_cplt_semphr, ticks_to_wait);
            if (pdTRUE != ret)
            {
                return -ETIMEDOUT;
            }
        }

        vPortFree(work_area);
    }

    return 0;
}

int sdcard_erase_blocks(uint32_t block_addr, uint16_t block_num)
{
    assert(block_num);
    HAL_StatusTypeDef hal_status;
    TimeOut_t timeout;
    TickType_t ticks_to_wait = pdMS_TO_TICKS(2 * SDCARD_DMA_BLOCK_TRANSFER_TIMEOUT_MS);
    vTaskSetTimeOutState(&timeout);
    BaseType_t timedout = xTaskCheckForTimeOut(&timeout, &ticks_to_wait);

    while ((HAL_SD_CARD_TRANSFER != HAL_SD_GetCardState(&h_sdio)) && (pdFALSE == timedout))
    {
        vTaskDelay(pdMS_TO_TICKS(1));
        timedout = xTaskCheckForTimeOut(&timeout, &ticks_to_wait);
    }

    if (pdTRUE == timedout)
    {
        return -ETIMEDOUT;
    }

    hal_status = HAL_SD_Erase(&h_sdio, block_addr, block_addr + (uint32_t)block_num);
    if (HAL_OK != hal_status)
    {
        return sd_error_to_errno(h_sdio.ErrorCode);
    }

    return 0;
}

uint64_t sdcard_get_capacity(void)
{
    HAL_SD_CardInfoTypeDef ci = {0};
    HAL_StatusTypeDef hal_status;

    hal_status = HAL_SD_GetCardInfo(&h_sdio, &ci);
    if (HAL_OK != hal_status)
    {
        return 0;
    }

    uint64_t capacity = ((uint64_t)ci.BlockNbr) * ((uint64_t)ci.BlockSize);
    return capacity;
}

/**
 * @brief Checks if a pointer is word-aligned.
 *
 * @param  pbuf Pointer to the memory buffer to be checked for alignment.
 *
 * @return True if the pointer is word-aligned, false otherwise.
 */
static bool is_word_aligned(const void *pbuf)
{
    assert(pbuf);
    return (0 == (((size_t)pbuf) & (sizeof(size_t) - 1))) ? true : false;
}

/**
 * @brief Initializes the SDIO interface.
 *
 * @return 0 on success,
 * @return < 0 on error.
 */
static int sdio_init(void)
{
    _error = HAL_OK;

    h_sdio.Instance = SDIO;
    h_sdio.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
    h_sdio.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
    h_sdio.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
    h_sdio.Init.BusWide = SDIO_BUS_WIDE_1B;
    h_sdio.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_ENABLE;
    h_sdio.Init.ClockDiv = SDIO_TRANSFER_CLK_DIV;

    HAL_StatusTypeDef ret;

    ret = HAL_SD_RegisterCallback(&h_sdio, HAL_SD_MSP_INIT_CB_ID, sdio_msp_init);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_SD_RegisterCallback(&h_sdio, HAL_SD_MSP_DEINIT_CB_ID, sdio_msp_deinit);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_SD_Init(&h_sdio);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    if (HAL_OK != _error)
    {
        return hal_statustypedef_to_errno(_error);
    }

    ret = HAL_SD_RegisterCallback(&h_sdio, HAL_SD_TX_CPLT_CB_ID, sdio_tx_cplt_callback);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_SD_RegisterCallback(&h_sdio, HAL_SD_RX_CPLT_CB_ID, sdio_rx_cplt_callback);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_SD_RegisterCallback(&h_sdio, HAL_SD_ERROR_CB_ID, sdio_error_callback);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_SD_ConfigWideBusOperation(&h_sdio, SDIO_BUS_WIDE_4B);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    return 0;
}

/**
 * @brief Initializes the SDIO MSP (low-level).
 *
 * @param h_sd Pointer to the SD handle structure.
 */
static void sdio_msp_init(SD_HandleTypeDef *h_sd)
{
    _error = sdio_clock_source_init();
    if (HAL_OK != _error)
    {
        return;
    }

    __HAL_RCC_SDIO_CLK_ENABLE();
    __HAL_RCC_SDIO_FORCE_RESET();
    __HAL_RCC_SDIO_RELEASE_RESET();

    sdcard_cmd_pin_init();
    sdcard_clk_pin_init();
    sdcard_d3_pin_init();
    sdcard_d2_pin_init();
    sdcard_d1_pin_init();
    sdcard_d0_pin_init();

    _error = sdcard_sdio_dma_rx_init(h_sd);
    if (HAL_OK != _error)
    {
        return;
    }

    _error = sdcard_sdio_dma_tx_init(h_sd);
    if (HAL_OK != _error)
    {
        return;
    }

    HAL_NVIC_SetPriority(SDIO_IRQn, SDCARD_SDIO_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(SDIO_IRQn);
}

/**
 * @brief De-initializes the SDIO interface.
 *
 * @return 0 on success,
 * @return < 0 on error.
 */
static int sdio_deinit(void)
{
    HAL_StatusTypeDef ret;

    ret = HAL_SD_UnRegisterCallback(&h_sdio, HAL_SD_TX_CPLT_CB_ID);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_SD_UnRegisterCallback(&h_sdio, HAL_SD_RX_CPLT_CB_ID);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_SD_UnRegisterCallback(&h_sdio, HAL_SD_ERROR_CB_ID);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_SD_DeInit(&h_sdio);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    if (HAL_OK != _error)
    {
        return hal_statustypedef_to_errno(_error);
    }

    ret = HAL_SD_UnRegisterCallback(&h_sdio, HAL_SD_MSP_INIT_CB_ID);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_SD_UnRegisterCallback(&h_sdio, HAL_SD_MSP_DEINIT_CB_ID);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    return 0;
}

/**
 * @brief De-initializes the SDIO MSP (low-level).
 *
 * @param h_sd Pointer to the SD handle structure.
 */
static void sdio_msp_deinit(SD_HandleTypeDef *h_sd)
{
    __HAL_RCC_SDIO_CLK_DISABLE();

    _error = sdio_clock_source_deinit();
    if (HAL_OK != _error)
    {
        return;
    }

    _error = clk48_clock_deinit();
    if (HAL_OK != _error)
    {
        return;
    }

    sdcard_cmd_pin_deinit();
    sdcard_clk_pin_deinit();
    sdcard_d3_pin_deinit();
    sdcard_d2_pin_deinit();
    sdcard_d1_pin_deinit();
    sdcard_d0_pin_deinit();

    _error = sdcard_sdio_dma_rx_deinit(h_sd);
    if (HAL_OK != _error)
    {
        return;
    }

    _error = sdcard_sdio_dma_tx_deinit(h_sd);
    if (HAL_OK != _error)
    {
        return;
    }

    HAL_NVIC_DisableIRQ(SDIO_IRQn);
}

/**
 * @brief SDIO Transmit Complete Callback
 *
 * @param h_sd Pointer to the SD handle structure (unused).
 */
static void sdio_tx_cplt_callback(SD_HandleTypeDef *h_sd)
{
    (void)h_sd;
    BaseType_t higher_priority_task_woken = pdFALSE;
    xSemaphoreGiveFromISR(_tx_cplt_semphr, &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

/**
 * @brief SDIO Receive Complete Callback
 *
 * @param h_sd Pointer to the SD handle structure (unused).
 */
static void sdio_rx_cplt_callback(SD_HandleTypeDef *h_sd)
{
    (void)h_sd;
    BaseType_t higher_priority_task_woken = pdFALSE;
    xSemaphoreGiveFromISR(_rx_cplt_semphr, &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

/**
 * @brief SDIO Error Callback
 *
 * @param h_sd Pointer to the SD handle structure (unused).
 */
static void sdio_error_callback(SD_HandleTypeDef *h_sd)
{
    (void)h_sd;
    error_handler();
}

/**
 * @brief SDIO Error Handler
 */
static void error_handler(void)
{
    //TODO: proper error handling
    //sdio_deinit();
    //sdio_init();
}

/**
 * @brief Converts an SD error code to a POSIC errno value.
 *
 * This function maps the given SD error code to a POSIX errno value
 * according to the POSIX error code definitions.
 *
 * @param  error The SD error code to be converted.
 * @return The corresponding standard errno value.
 */
static int sd_error_to_errno(const uint32_t error)
{
    switch (error)
    {
        case HAL_SD_ERROR_NONE                   : return 0;

        case HAL_SD_ERROR_PARAM                  :
        case HAL_SD_ERROR_ADDR_MISALIGNED        :  /* not used by HAL */
        case HAL_SD_ERROR_BLOCK_LEN_ERR          :  /* not used by HAL */
        case HAL_SD_ERROR_ADDR_OUT_OF_RANGE      : return -EINVAL;

        case HAL_SD_ERROR_CMD_RSP_TIMEOUT        :  /* not used by HAL */
        case HAL_SD_ERROR_DATA_TIMEOUT           :
        case HAL_SD_ERROR_TIMEOUT                : return -ETIMEDOUT;

        case HAL_SD_ERROR_CMD_CRC_FAIL           :  /* not used by HAL */
        case HAL_SD_ERROR_DATA_CRC_FAIL          :
        case HAL_SD_ERROR_COM_CRC_FAILED         : return -EBADMSG; /* not used by HAL */

        case HAL_SD_ERROR_TX_UNDERRUN            :
        case HAL_SD_ERROR_RX_OVERRUN             :
        case HAL_SD_ERROR_STREAM_WRITE_OVERRUN   :  /* not used by HAL */
        case HAL_SD_ERROR_STREAM_READ_UNDERRUN   : return -ENOMEM;  /* not used by HAL */

        case HAL_SD_ERROR_BUSY                   : return -EBUSY;

        case HAL_SD_ERROR_REQUEST_NOT_APPLICABLE :
        case HAL_SD_ERROR_UNSUPPORTED_FEATURE    : return -ENOTSUP;

        case HAL_SD_ERROR_INVALID_VOLTRANGE      :
        case HAL_SD_ERROR_DMA                    : return -EIO;

        case HAL_SD_ERROR_GENERAL_UNKNOWN_ERR    :  /* not used by HAL */
        case HAL_SD_ERROR_ERASE_SEQ_ERR          :  /* not used by HAL */
        case HAL_SD_ERROR_AKE_SEQ_ERR            :  /* not used by HAL */
        case HAL_SD_ERROR_WRITE_PROT_VIOLATION   :  /* not used by HAL */
        case HAL_SD_ERROR_LOCK_UNLOCK_FAILED     :
        case HAL_SD_ERROR_CARD_ECC_FAILED        :  /* not used by HAL */
        case HAL_SD_ERROR_BAD_ERASE_PARAM        :  /* not used by HAL */
        case HAL_SD_ERROR_ILLEGAL_CMD            :  /* not used by HAL */
        case HAL_SD_ERROR_CC_ERR                 :  /* not used by HAL */
        case HAL_SD_ERROR_CID_CSD_OVERWRITE      :  /* not used by HAL */
        case HAL_SD_ERROR_WP_ERASE_SKIP          :  /* not used by HAL */
        case HAL_SD_ERROR_CARD_ECC_DISABLED      :  /* not used by HAL */
        case HAL_SD_ERROR_ERASE_RESET            : return -EFAULT;  /* not used by HAL */
    }

    return (int)error;
}

/**
 * @brief SDIO Interrupt Handler
 */
void SDIO_IRQHandler(void)
{
    HAL_SD_IRQHandler(&h_sdio);
}
/** @} */
