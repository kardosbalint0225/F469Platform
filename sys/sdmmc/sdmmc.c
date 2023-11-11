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
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

EXTI_HandleTypeDef hexti_linex;
SD_HandleTypeDef h_sdio;
DMA_HandleTypeDef h_sdio_dma_tx;
DMA_HandleTypeDef h_sdio_dma_rx;

static SemaphoreHandle_t h_sdio_tx_cplt_semphr = NULL;
static StaticSemaphore_t sdio_tx_cplt_semphr_storage;
static SemaphoreHandle_t h_sdio_rx_cplt_semphr = NULL;
static StaticSemaphore_t sdio_rx_cplt_semphr_storage;

static StackType_t sdmmc_mount_task_stack[SDMMC_MOUNT_TASK_STACKSIZE];
static StaticTask_t sdmmc_mount_task_tcb;
static TaskHandle_t h_sdmmc_mount_task = NULL;

static bool is_mounted = false;

enum _SDMMC_MOUNT_TASK_NOTIFICATION
{
    SDMMC_MOUNT_TASK_NOTIFICATION_CARD_DETECT_CHANGED = 0x00000001,
    SDMMC_MOUNT_TASK_NOTIFICATION_INT_MAX             = 0x7FFFFFFF,
};

enum _SDMMC_CARD_PRESENCE_STATE
{
    SDMMC_CARD_PRESENCE_STATE_INSERTED = 0x00000001,
    SDMMC_CARD_PRESENCE_STATE_REMOVED  = 0x00000002,
    SDMMC_CARD_PRESENCE_STATE_UNSTABLE = 0x00000003,
    SDMMC_CARD_PRESENCE_STATE_INT_MAX  = 0x7FFFFFFF,
};

static int sdio_init(void);
static int sdio_deinit(void);
static void sdio_msp_init(SD_HandleTypeDef *h_sd);
static void sdio_msp_deinit(SD_HandleTypeDef *h_sd);
static void sdio_tx_cplt_callback(SD_HandleTypeDef *h_sd);
static void sdio_rx_cplt_callback(SD_HandleTypeDef *h_sd);
static int sdio_cd_pin_init(void);
static int sdio_cd_pin_deinit(void);
static void exti_linex_callback(void);
static void wait_for_stable_cd_pin_signal(const uint32_t timeout_ms, uint32_t *state);
static void sdmmc_mount_task(void *params);

static void exti_linex_callback(void)
{
    portBASE_TYPE higher_priority_task_woken = pdFALSE;
    BaseType_t ret = xTaskNotifyFromISR(h_sdmmc_mount_task,
                                        (uint32_t)SDMMC_MOUNT_TASK_NOTIFICATION_CARD_DETECT_CHANGED,
                                        eSetBits,
                                        &higher_priority_task_woken);
    assert_param(pdPASS == ret);
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

static void wait_for_stable_cd_pin_signal(const uint32_t timeout_ms, uint32_t *state)
{
    TimeOut_t timeout;
    TickType_t ticks_to_wait = pdMS_TO_TICKS(timeout_ms);
    vTaskSetTimeOutState(&timeout);

    uint32_t signal = 0xAAAAAAAAUL;
    bool is_stable = false;
    BaseType_t timedout = xTaskCheckForTimeOut(&timeout, &ticks_to_wait);

    while ((false == is_stable) && (pdFALSE == timedout))
    {
        const GPIO_PinState cd_pin = HAL_GPIO_ReadPin(SDMMC_CD_PIN_GPIO_PORT, SDMMC_CD_PIN);
        signal <<= 1;
        signal |= cd_pin == GPIO_PIN_RESET ? 0 : 1;
        is_stable = (0 == signal) || (ULONG_MAX == signal) ? true : false;
        timedout = xTaskCheckForTimeOut(&timeout, &ticks_to_wait);
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    if (pdTRUE == timedout)
    {
        *state = (uint32_t)SDMMC_CARD_PRESENCE_STATE_UNSTABLE;
    }
    else if (true == is_stable)
    {
        *state = 0 == signal ? (uint32_t)SDMMC_CARD_PRESENCE_STATE_INSERTED : (uint32_t)SDMMC_CARD_PRESENCE_STATE_REMOVED;
    }
}

static void sdmmc_mount_task(void *params)
{
    (void)params;

    uint32_t sdmmc_card_presence_state;
    wait_for_stable_cd_pin_signal(SDMMC_CD_PIN_DEBOUNCE_TIMEOUT_MS, &sdmmc_card_presence_state);
    if ((uint32_t)SDMMC_CARD_PRESENCE_STATE_INSERTED == sdmmc_card_presence_state)
    {
        //TODO: vfs_mount
        sdio_init();
        printf("sdmmc card is mounted\r\n");
        is_mounted = true;
        sdmmc_get_capacity();
    }

    BaseType_t ret;

    for ( ;; )
    {
        uint32_t notification;

        ret = xTaskNotifyWait(0, 0, &notification, portMAX_DELAY);
        assert_param(pdPASS == ret);

        if ((uint32_t)SDMMC_MOUNT_TASK_NOTIFICATION_CARD_DETECT_CHANGED == notification)
        {
            wait_for_stable_cd_pin_signal(SDMMC_CD_PIN_DEBOUNCE_TIMEOUT_MS, &sdmmc_card_presence_state);

            switch(sdmmc_card_presence_state)
            {
                case (uint32_t)SDMMC_CARD_PRESENCE_STATE_INSERTED :
                {
                    if (false == is_mounted)
                    {
                        //TODO: vfs_mount
                        sdio_init();
                        printf("sdmmc card is mounted\r\n");
                        is_mounted = true;
                    }
                    else
                    {
                        printf("The memory card is not inserted properly.\r\n");
                    }
                }
                break;

                case (uint32_t)SDMMC_CARD_PRESENCE_STATE_REMOVED :
                {
                    if (true == is_mounted)
                    {
                        //TODO: vfs_unmount
                        sdio_deinit();
                        printf("sdmmc card is unmounted\r\n");
                        is_mounted = false;
                    }
                    else
                    {
                        printf("The memory card is not inserted properly.\r\n");
                    }
                }
                break;

                case (uint32_t)SDMMC_CARD_PRESENCE_STATE_UNSTABLE :
                {
                    printf("The memory card is not inserted properly.\r\n");
                }
                break;

                default:
                {
                    assert_param(0);
                }
                break;
            }
        }

        xTaskNotifyWait(0, ULONG_MAX, &notification, 0);
    }
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

static int sdio_cd_pin_init(void)
{
    SDMMC_CD_PIN_GPIO_CLK_ENABLE();

    GPIO_InitTypeDef card_detect_pin_config = {
        .Pin = SDMMC_CD_PIN,
        .Mode = GPIO_MODE_IT_RISING_FALLING,
        .Pull = GPIO_PULLUP,
    };
    HAL_GPIO_Init(SDMMC_CD_PIN_GPIO_PORT, &card_detect_pin_config);

    EXTI_ConfigTypeDef exti_config = {
        .Line = SDMMC_CD_PIN_EXTI_LINE,
        .Mode = EXTI_MODE_INTERRUPT,
        .Trigger = EXTI_TRIGGER_RISING_FALLING,
        .GPIOSel = SDMMC_CD_PIN_EXTI_GPIO
    };

    HAL_StatusTypeDef ret;
    ret = HAL_EXTI_SetConfigLine(&hexti_linex, &exti_config);
    assert_param(HAL_OK == ret);

    ret = HAL_EXTI_RegisterCallback(&hexti_linex, HAL_EXTI_COMMON_CB_ID, exti_linex_callback);
    assert_param(HAL_OK == ret);

    HAL_NVIC_SetPriority(SDMMC_CD_PIN_EXTIx_IRQn, SDMMC_CD_PIN_EXTIx_IRQ_PRIORITY, 0U);
    HAL_NVIC_EnableIRQ(SDMMC_CD_PIN_EXTIx_IRQn);

    return 0;
}

static int sdio_cd_pin_deinit(void)
{
    HAL_StatusTypeDef ret;

    HAL_NVIC_DisableIRQ(SDMMC_CD_PIN_EXTIx_IRQn);

    ret = HAL_EXTI_ClearConfigLine(&hexti_linex);
    assert_param(HAL_OK == ret);

    HAL_GPIO_DeInit(SDMMC_CD_PIN_GPIO_PORT, SDMMC_CD_PIN);

    return 0;
}

int sdmmc_init(void)
{
    h_sdio_tx_cplt_semphr = xSemaphoreCreateBinaryStatic(&sdio_tx_cplt_semphr_storage);
    assert_param(NULL != h_sdio_tx_cplt_semphr);
    h_sdio_rx_cplt_semphr = xSemaphoreCreateBinaryStatic(&sdio_rx_cplt_semphr_storage);
    assert_param(NULL != h_sdio_rx_cplt_semphr);

    h_sdmmc_mount_task = xTaskCreateStatic(sdmmc_mount_task,
                                           "SDMMC Mount",
                                           SDMMC_MOUNT_TASK_STACKSIZE,
                                           NULL,
                                           SDMMC_MOUNT_TASK_PRIORITY,
                                           sdmmc_mount_task_stack,
                                           &sdmmc_mount_task_tcb);
    assert_param(NULL != h_sdmmc_mount_task);

    is_mounted = false;

    sdio_cd_pin_init();

    return 0;
}

int sdmmc_deinit(void)
{
    vTaskDelete(h_sdmmc_mount_task);
    vSemaphoreDelete(h_sdio_tx_cplt_semphr);
    vSemaphoreDelete(h_sdio_rx_cplt_semphr);

    h_sdmmc_mount_task = NULL;
    h_sdio_tx_cplt_semphr = NULL;
    h_sdio_rx_cplt_semphr = NULL;

    sdio_cd_pin_deinit();

    return 0;
}

int sdmmc_card_init(void)
{
    return 0;
}

int sdmmc_read_blocks(uint32_t block_addr,
                      uint16_t block_size,
                      uint16_t block_num,
                      void *data,
                      uint16_t *done)
{
return 0;
}

int sdmmc_write_blocks(uint32_t block_addr,
                       uint16_t block_size,
                       uint16_t block_num,
                       const void *data,
                       uint16_t *done)
{
return 0;
}

int sdmmc_erase_blocks(uint32_t block_addr, uint16_t block_num)
{
    (void)block_addr;
    (void)block_num;

    return 0;
}

uint64_t sdmmc_get_capacity(void)
{
    HAL_SD_CardInfoTypeDef ci = {0};
    HAL_StatusTypeDef ret;
    ret = HAL_SD_GetCardInfo(&h_sdio, &ci);
    assert_param(HAL_OK == ret);
    printf("CardType     : %lu\r\n", ci.CardType);
    printf("CardVersion  : %lu\r\n", ci.CardVersion);
    printf("Class        : %lu\r\n", ci.Class);
    printf("RelCardAdd   : %lu\r\n", ci.RelCardAdd);
    printf("BlockNbr     : %lu\r\n", ci.BlockNbr);
    printf("BlockSize    : %lu\r\n", ci.BlockSize);
    printf("LogBlockNbr  : %lu\r\n", ci.LogBlockNbr);
    printf("LogBlockSize : %lu\r\n", ci.LogBlockSize);
    return 0;
}

