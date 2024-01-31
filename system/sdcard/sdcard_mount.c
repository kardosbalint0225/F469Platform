/*
 * sdcard_mount.c
 *
 *  Created on: 2023. nov. 13.
 *      Author: Balint
 */
#include "sdcard_mount.h"
#include "sdcard_config.h"
#include "stm32f4xx_hal.h"
#include "gpio.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <limits.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "fs/fatfs.h"
#include "vfs.h"

#include "mtd.h"
#include "mtd_sdcard.h"

#define SDCARD_MOUNT_PATH  "/sd"

static fatfs_desc_t _fatfs_desc;
static vfs_mount_t _fatfs_sdcard_vfs_mount = {
    .mount_point = SDCARD_MOUNT_PATH,
    .fs = &fatfs_file_system,
    .private_data = (void *)&_fatfs_desc,
};
static mtd_sdcard_t mtd_sdcard;

static StackType_t _sdcard_mount_task_stack[SDCARD_MOUNT_TASK_STACKSIZE];
static StaticTask_t _sdcard_mount_task_tcb;
static TaskHandle_t h_sdcard_mount_task = NULL;

enum _SDCARD_MOUNT_TASK_NOTIFICATION
{
    SDCARD_MOUNT_TASK_NOTIFICATION_CARD_DETECT_CHANGED = 0x00000001ul,
    SDCARD_MOUNT_TASK_NOTIFICATION_INT_MAX             = 0x7FFFFFFFul,
};

enum _SDCARD_CARD_PRESENCE_STATE
{
    SDCARD_CARD_PRESENCE_STATE_INSERTED = 0x00000001ul,
    SDCARD_CARD_PRESENCE_STATE_REMOVED  = 0x00000002ul,
    SDCARD_CARD_PRESENCE_STATE_UNSTABLE = 0x00000003ul,
    SDCARD_CARD_PRESENCE_STATE_INT_MAX  = 0x7FFFFFFFul,
};

static void exti_sdcard_cd_pin_callback(void);
static void wait_for_stable_cd_pin_signal(const uint32_t timeout_ms, uint32_t *state);
static void sdcard_mount_task(void *params);
static int sdcard_mount(void);
static int sdcard_unmount(void);

int sdcard_mount_init(void)
{
    h_sdcard_mount_task = xTaskCreateStatic(sdcard_mount_task,
                                            "SDCARD Mount",
                                            SDCARD_MOUNT_TASK_STACKSIZE,
                                            NULL,
                                            SDCARD_MOUNT_TASK_PRIORITY,
                                            _sdcard_mount_task_stack,
                                            &_sdcard_mount_task_tcb);
    assert(h_sdcard_mount_task);

    return sdcard_cd_pin_init(exti_sdcard_cd_pin_callback);
}

int sdcard_mount_deinit(void)
{
    vTaskDelete(h_sdcard_mount_task);
    h_sdcard_mount_task = NULL;

    return sdcard_cd_pin_deinit();
}

static void sdcard_mount_task(void *params)
{
    (void)params;

    bool is_mounted = false;
    uint32_t card_presence_state;

    wait_for_stable_cd_pin_signal(SDCARD_CD_PIN_DEBOUNCE_TIMEOUT_MS, &card_presence_state);

    if ((uint32_t)SDCARD_CARD_PRESENCE_STATE_INSERTED == card_presence_state)
    {
        if (0 == sdcard_mount())
        {
            is_mounted = true;
        }
    }

    BaseType_t ret;

    for ( ;; )
    {
        uint32_t notification;

        ret = xTaskNotifyWait(0, 0, &notification, portMAX_DELAY);
        assert(ret);

        if ((uint32_t)SDCARD_MOUNT_TASK_NOTIFICATION_CARD_DETECT_CHANGED == notification)
        {
            wait_for_stable_cd_pin_signal(SDCARD_CD_PIN_DEBOUNCE_TIMEOUT_MS, &card_presence_state);

            switch(card_presence_state)
            {
                case (uint32_t)SDCARD_CARD_PRESENCE_STATE_INSERTED :
                {
                    if (false == is_mounted)
                    {
                        if (0 == sdcard_mount())
                        {
                            is_mounted = true;
                            printf("sdcard card is mounted\r\n");
                        }
                    }
                    else
                    {
                        printf("The memory card is not inserted properly.\r\n");
                    }
                }
                break;

                case (uint32_t)SDCARD_CARD_PRESENCE_STATE_REMOVED :
                {
                    if (true == is_mounted)
                    {
                        if (0 == sdcard_unmount())
                        {
                            is_mounted = false;
                            printf("sdcard card is unmounted\r\n");
                        }
                    }
                    else
                    {
                        printf("The memory card is not inserted properly.\r\n");
                    }
                }
                break;

                case (uint32_t)SDCARD_CARD_PRESENCE_STATE_UNSTABLE :
                {
                    printf("The memory card is not inserted properly.\r\n");
                }
                break;

                default:
                {
                    assert(0);
                }
                break;
            }
        }

        xTaskNotifyWait(0, ULONG_MAX, &notification, 0);
    }
}

static int sdcard_mount(void)
{
    int err;

    mtd_sdcard.base.driver = &mtd_sdcard_driver;
    _fatfs_desc.dev = (mtd_dev_t *)&mtd_sdcard;

    err = vfs_mount(&_fatfs_sdcard_vfs_mount);
    printf("\r\n  sdcard_mount : %s\r\n", strerror(-err));

    return err;
}

static int sdcard_unmount(void)
{
    int err;

    err = vfs_umount(&_fatfs_sdcard_vfs_mount, true);
    printf("\r\n  sdcard_unmount : %s\r\n", strerror(-err));
    if (err < 0)
    {
        return err;
    }

    err = sdcard_deinit();
    if (err < 0)
    {
        return err;
    }

    vPortFree(mtd_sdcard.base.work_area);

    mtd_sdcard.base.driver = NULL;
    _fatfs_desc.dev = NULL;

    return 0;
}

static void exti_sdcard_cd_pin_callback(void)
{
    BaseType_t higher_priority_task_woken = pdFALSE;
    xTaskNotifyFromISR(h_sdcard_mount_task,
                       (uint32_t)SDCARD_MOUNT_TASK_NOTIFICATION_CARD_DETECT_CHANGED,
                       eSetBits,
                       &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

static void wait_for_stable_cd_pin_signal(const uint32_t timeout_ms, uint32_t *state)
{
    TimeOut_t timeout;
    TickType_t ticks_to_wait = pdMS_TO_TICKS(timeout_ms);
    vTaskSetTimeOutState(&timeout);

    uint32_t signal = 0xAAAAAAAAul;
    bool is_stable = false;
    BaseType_t timedout = xTaskCheckForTimeOut(&timeout, &ticks_to_wait);

    while ((false == is_stable) && (pdFALSE == timedout))
    {
        const GPIO_PinState cd_pin = HAL_GPIO_ReadPin(SDCARD_CD_PIN_GPIO_PORT, SDCARD_CD_PIN);
        signal <<= 1;
        signal |= cd_pin == GPIO_PIN_RESET ? 0 : 1;
        is_stable = (0 == signal) || (ULONG_MAX == signal) ? true : false;
        timedout = xTaskCheckForTimeOut(&timeout, &ticks_to_wait);
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    if (pdTRUE == timedout)
    {
        *state = (uint32_t)SDCARD_CARD_PRESENCE_STATE_UNSTABLE;
    }
    else if (true == is_stable)
    {
        *state = 0 == signal ? (uint32_t)SDCARD_CARD_PRESENCE_STATE_INSERTED : (uint32_t)SDCARD_CARD_PRESENCE_STATE_REMOVED;
    }
}




