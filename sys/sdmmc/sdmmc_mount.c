/*
 * sdmmc_mount.c
 *
 *  Created on: 2023. nov. 13.
 *      Author: Balint
 */
#include "sdmmc_mount.h"
#include "sdmmc_config.h"
#include "stm32f4xx_hal.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "sdmmc.h"

EXTI_HandleTypeDef hexti_linex;

static StackType_t sdmmc_mount_task_stack[SDMMC_MOUNT_TASK_STACKSIZE];
static StaticTask_t sdmmc_mount_task_tcb;
static TaskHandle_t h_sdmmc_mount_task = NULL;

static bool is_mounted = false;

static uint8_t aligned_read_buffer[512ul * 16] __attribute__((aligned(8)));
static uint8_t unaligned_read_buffer[512ul * 16];

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

int sdmmc_cd_pin_init(void);
int sdmmc_cd_pin_deinit(void);
static void exti_linex_callback(void);
static void wait_for_stable_cd_pin_signal(const uint32_t timeout_ms, uint32_t *state);
static void sdmmc_mount_task(void *params);

int sdmmc_mount_init(void)
{
    h_sdmmc_mount_task = xTaskCreateStatic(sdmmc_mount_task,
                                           "SDMMC Mount",
                                           SDMMC_MOUNT_TASK_STACKSIZE,
                                           NULL,
                                           SDMMC_MOUNT_TASK_PRIORITY,
                                           sdmmc_mount_task_stack,
                                           &sdmmc_mount_task_tcb);
    assert_param(NULL != h_sdmmc_mount_task);

    is_mounted = false;

    sdmmc_cd_pin_init();

    return 0;
}

int sdmmc_mount_deinit(void)
{
    vTaskDelete(h_sdmmc_mount_task);
    h_sdmmc_mount_task = NULL;

    sdmmc_cd_pin_deinit();

    return 0;
}

static void sdmmc_mount_task(void *params)
{
    (void)params;

    uint32_t sdmmc_card_presence_state;
    wait_for_stable_cd_pin_signal(SDMMC_CD_PIN_DEBOUNCE_TIMEOUT_MS, &sdmmc_card_presence_state);
    if ((uint32_t)SDMMC_CARD_PRESENCE_STATE_INSERTED == sdmmc_card_presence_state)
    {
        //TODO: vfs_mount
        sdmmc_init();
        printf("sdmmc card is mounted\r\n");
        is_mounted = true;
        sdmmc_get_capacity();

        uint32_t average = 0;
        TickType_t tick_start, tick_end;

        printf("aligned read:\r\n");
        for (int i = 0; i < 1000; i++)
        {
            tick_start = xTaskGetTickCount();
            sdmmc_read_blocks(0, 512ul, 15, aligned_read_buffer, NULL);
            tick_end = xTaskGetTickCount();
            //printf("tick_end - tick_start = %lu\r\n", tick_end - tick_start);
            average += tick_end - tick_start;
        }
        printf("aligned read avg = %f\r\n", ((float)average) / 1000.0f);

        average = 0;
        printf("unaligned read:\r\n");
        for (int i = 0; i < 1000; i++)
        {
            tick_start = xTaskGetTickCount();
            sdmmc_read_blocks(0, 512ul, 15, &unaligned_read_buffer[1], NULL);
            tick_end = xTaskGetTickCount();
            //printf("tick_end - tick_start = %lu\r\n", tick_end - tick_start);
            average += tick_end - tick_start;
        }
        printf("unaligned read avg = %f\r\n", ((float)average) / 1000.0f);

        for (uint32_t i = 0; i < (15 * 512ul); i++)
        {
            if (aligned_read_buffer[i] != unaligned_read_buffer[1 + i])
            {
                printf("i = %lu item mismatch!\r\n", i);
            }
        }

        printf("Tests finished running.\r\n");
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
                        sdmmc_init();
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
                        sdmmc_deinit();
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

int sdmmc_cd_pin_init(void)
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

int sdmmc_cd_pin_deinit(void)
{
    HAL_StatusTypeDef ret;

    HAL_NVIC_DisableIRQ(SDMMC_CD_PIN_EXTIx_IRQn);

    ret = HAL_EXTI_ClearConfigLine(&hexti_linex);
    assert_param(HAL_OK == ret);

    HAL_GPIO_DeInit(SDMMC_CD_PIN_GPIO_PORT, SDMMC_CD_PIN);

    return 0;
}

static void exti_linex_callback(void)
{
    BaseType_t higher_priority_task_woken = pdFALSE;
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




