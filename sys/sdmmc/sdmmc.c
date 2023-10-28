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
#include "timers.h"

EXTI_HandleTypeDef hexti2;

static StackType_t sdmmc_card_mount_task_stack[4*configMINIMAL_STACK_SIZE];
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
    SDMMC_CARD_PRESENCE_STATE_INT_MAX  = 0x7FFFFFFF,
};

static uint32_t sdmmc_card_detect_signal(void);

static void exti2_callback(void)
{
    portBASE_TYPE higher_priority_task_woken = pdFALSE;
    BaseType_t ret = xTaskNotifyFromISR(h_sdmmc_card_mount_task,
                                        (uint32_t)SDMMC_CARD_MOUNT_TASK_NOTIFICATION_CARD_DETECT_CHANGED,
                                        eSetBits,
                                        &higher_priority_task_woken);
    assert_param(pdPASS == ret);
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0')


static void sdmmc_card_mount_task(void *params)
{
    (void)params;

    BaseType_t ret;

    for ( ;; )
    {
        uint32_t notification;

        ret = xTaskNotifyWait(0, 0, &notification, portMAX_DELAY);
        assert_param(pdPASS == ret);

        if ((uint32_t)SDMMC_CARD_MOUNT_TASK_NOTIFICATION_CARD_DETECT_CHANGED == notification)
        {
            vTaskDelay(pdMS_TO_TICKS(SDMMC_CARD_DETECT_DEBOUNCE_TIME_MS));

            TimeOut_t timeout;
            TickType_t ticks_to_wait = pdMS_TO_TICKS(SDMMC_CARD_DETECT_DEBOUNCE_TIMEOUT_MS);
            vTaskSetTimeOutState(&timeout);

            uint32_t card_detect_signal = 0x00010001;
            bool stable_signal_is_detected = false;
            int cnt = 0;
            uint32_t sdmmc_card_presence_state;

            while (pdTRUE != xTaskCheckForTimeOut(&timeout, &ticks_to_wait) && true != stable_signal_is_detected)
            {
                card_detect_signal <<= 1;
                card_detect_signal |= sdmmc_card_detect_signal();
                vTaskDelay(pdMS_TO_TICKS(30));
                printf("%02d card_detect_signal:  "BYTE_TO_BINARY_PATTERN" "
                                                 ""BYTE_TO_BINARY_PATTERN" "
                                                 ""BYTE_TO_BINARY_PATTERN" "
                                                 ""BYTE_TO_BINARY_PATTERN"\r\n",
                                                 cnt++,
                                                 BYTE_TO_BINARY(card_detect_signal >> 24),
                                                 BYTE_TO_BINARY(card_detect_signal >> 16),
                                                 BYTE_TO_BINARY(card_detect_signal >> 8),
                                                 BYTE_TO_BINARY(card_detect_signal));
                if (card_detect_signal == 0 || card_detect_signal == ULONG_MAX)
                {
                    stable_signal_is_detected = true;
                    if (card_detect_signal == ULONG_MAX)
                    {
                        sdmmc_card_presence_state = (uint32_t)SDMMC_CARD_PRESENCE_STATE_INSERTED;
                    }
                    else
                    {
                        sdmmc_card_presence_state = (uint32_t)SDMMC_CARD_PRESENCE_STATE_REMOVED;
                    }
                }
            }

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
                        printf("inserted->mounted?\r\n");
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
                        printf("removeded->not mounted?\r\n");
                    }
                }
                break;

                default:
                {
                    assert_param(0);
                }
                break;
            }
            xTaskNotifyWait(0, ULONG_MAX, &notification, 0);
        }
        else
        {
            assert_param(0);
        }
    }
}

static uint32_t sdmmc_card_detect_signal(void)
{
    const GPIO_PinState cd_pin = HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_2);
    return GPIO_PIN_RESET == cd_pin ? 1 : 0;
}

int sdmmc_init(void)
{
    h_sdmmc_card_mount_task = xTaskCreateStatic(sdmmc_card_mount_task,
                                                "SDMMC Mount",
                                                4*configMINIMAL_STACK_SIZE,
                                                NULL,
                                                3,
                                                sdmmc_card_mount_task_stack,
                                                &sdmmc_card_mount_task_tcb);
    assert_param(NULL != h_sdmmc_card_mount_task);


    __HAL_RCC_GPIOG_CLK_ENABLE();

    GPIO_InitTypeDef card_detect_gpio_config = {
        .Pin = GPIO_PIN_2,
        .Mode = GPIO_MODE_IT_RISING_FALLING,
        .Pull = GPIO_PULLUP,
    };
    HAL_GPIO_Init(GPIOG, &card_detect_gpio_config);

    EXTI_ConfigTypeDef exti2_config = {
        .Line = EXTI_LINE_2,
        .Mode = EXTI_MODE_INTERRUPT,
        .Trigger = EXTI_TRIGGER_RISING_FALLING,
        .GPIOSel = EXTI_GPIOG
    };

    HAL_StatusTypeDef ret;
    ret = HAL_EXTI_SetConfigLine(&hexti2, &exti2_config);
    assert_param(HAL_OK == ret);

    ret = HAL_EXTI_RegisterCallback(&hexti2, HAL_EXTI_COMMON_CB_ID, exti2_callback);
    assert_param(HAL_OK == ret);

    HAL_NVIC_SetPriority(EXTI2_IRQn, 10, 0U);
    HAL_NVIC_EnableIRQ(EXTI2_IRQn);

    return 0;
}

int sdmmc_deinit(void)
{
    return 0;
}

