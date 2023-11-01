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

static StackType_t sdmmc_card_mount_task_stack[SDMMC_CARD_MOUNT_TASK_STACKSIZE];
static StaticTask_t sdmmc_card_mount_task_tcb;
static TaskHandle_t h_sdmmc_card_mount_task = NULL;

static bool is_mounted = false;

enum _SDMMC_CARD_MOUNT_TASK_NOTIFICATION
{
    SDMMC_CARD_MOUNT_TASK_NOTIFICATION_CARD_DETECT_CHANGED = 0x00000001,
    SDMMC_CARD_MOUNT_TASK_NOTIFICATION_INT_MAX             = 0x7FFFFFFF,
};

enum _SDMMC_CARD_PRESENCE_STATE
{
    SDMMC_CARD_PRESENCE_STATE_INSERTED = 0x00000001,
    SDMMC_CARD_PRESENCE_STATE_REMOVED  = 0x00000002,
    SDMMC_CARD_PRESENCE_STATE_UNSTABLE = 0x00000003,
    SDMMC_CARD_PRESENCE_STATE_INT_MAX  = 0x7FFFFFFF,
};

static void exti_linex_callback(void)
{
    portBASE_TYPE higher_priority_task_woken = pdFALSE;
    BaseType_t ret = xTaskNotifyFromISR(h_sdmmc_card_mount_task,
                                        (uint32_t)SDMMC_CARD_MOUNT_TASK_NOTIFICATION_CARD_DETECT_CHANGED,
                                        eSetBits,
                                        &higher_priority_task_woken);
    assert_param(pdPASS == ret);
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

static void wait_for_stable_card_detect_pin_signal(const uint32_t timeout_ms, uint32_t *state)
{
    TimeOut_t timeout;
    TickType_t ticks_to_wait = pdMS_TO_TICKS(timeout_ms);
    vTaskSetTimeOutState(&timeout);

    uint32_t signal = 0xAAAAAAAA;
    bool is_stable = false;
    BaseType_t timedout = xTaskCheckForTimeOut(&timeout, &ticks_to_wait);

    while ((false == is_stable) && (pdFALSE == timedout))
    {
        const GPIO_PinState cd_pin = HAL_GPIO_ReadPin(SDMMC_CARD_DETECT_GPIO_PORT, SDMMC_CARD_DETECT_GPIO_PIN);
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

static void sdmmc_card_mount_task(void *params)
{
    (void)params;

    uint32_t sdmmc_card_presence_state;
    wait_for_stable_card_detect_pin_signal(SDMMC_CARD_DETECT_DEBOUNCE_TIMEOUT_MS, &sdmmc_card_presence_state);
    if ((uint32_t)SDMMC_CARD_PRESENCE_STATE_INSERTED == sdmmc_card_presence_state)
    {
        //TODO: vfs_mount
        printf("sdmmc card is mounted\r\n");
        is_mounted = true;
    }

    BaseType_t ret;

    for ( ;; )
    {
        uint32_t notification;

        ret = xTaskNotifyWait(0, 0, &notification, portMAX_DELAY);
        assert_param(pdPASS == ret);

        if ((uint32_t)SDMMC_CARD_MOUNT_TASK_NOTIFICATION_CARD_DETECT_CHANGED == notification)
        {
            wait_for_stable_card_detect_pin_signal(SDMMC_CARD_DETECT_DEBOUNCE_TIMEOUT_MS, &sdmmc_card_presence_state);

            switch(sdmmc_card_presence_state)
            {
                case (uint32_t)SDMMC_CARD_PRESENCE_STATE_INSERTED :
                {
                    if (false == is_mounted)
                    {
                        //TODO: vfs_mount
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

int sdmmc_init(void)
{
    h_sdmmc_card_mount_task = xTaskCreateStatic(sdmmc_card_mount_task,
                                                "SDMMC Mount",
                                                SDMMC_CARD_MOUNT_TASK_STACKSIZE,
                                                NULL,
                                                SDMMC_CARD_MOUNT_TASK_PRIORITY,
                                                sdmmc_card_mount_task_stack,
                                                &sdmmc_card_mount_task_tcb);
    assert_param(NULL != h_sdmmc_card_mount_task);

    is_mounted = false;

    SDMMC_CARD_DETECT_GPIO_CLK_ENABLE();

    GPIO_InitTypeDef card_detect_pin_config = {
        .Pin = SDMMC_CARD_DETECT_GPIO_PIN,
        .Mode = GPIO_MODE_IT_RISING_FALLING,
        .Pull = GPIO_PULLUP,
    };
    HAL_GPIO_Init(SDMMC_CARD_DETECT_GPIO_PORT, &card_detect_pin_config);

    EXTI_ConfigTypeDef exti_config = {
        .Line = SDMMC_CARD_DETECT_EXTI_LINE,
        .Mode = EXTI_MODE_INTERRUPT,
        .Trigger = EXTI_TRIGGER_RISING_FALLING,
        .GPIOSel = SDMMC_CARD_DETECT_EXTI_GPIO
    };

    HAL_StatusTypeDef ret;
    ret = HAL_EXTI_SetConfigLine(&hexti_linex, &exti_config);
    assert_param(HAL_OK == ret);

    ret = HAL_EXTI_RegisterCallback(&hexti_linex, HAL_EXTI_COMMON_CB_ID, exti_linex_callback);
    assert_param(HAL_OK == ret);

    HAL_NVIC_SetPriority(SDMMC_CARD_DETECT_EXTIx_IRQn, SDMMC_CARD_DETECT_EXTIx_IRQ_PRIORITY, 0U);
    HAL_NVIC_EnableIRQ(SDMMC_CARD_DETECT_EXTIx_IRQn);

    return 0;
}

int sdmmc_deinit(void)
{
    return 0;
}

