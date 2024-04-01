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
 * @ingroup     system_sdcard_monitor
 *
 * @file        sdcard_monitor.c
 * @brief       SD Card Monitor
 */
#include "sdcard_monitor.h"
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

static StackType_t _sdcard_monitor_task_stack[SDCARD_MONITOR_TASK_STACKSIZE];
static StaticTask_t _sdcard_monitor_task_tcb;
static TaskHandle_t h_sdcard_monitor_task = NULL;

enum _SDCARD_MONITOR_TASK_NOTIFICATION
{
    SDCARD_MONITOR_TASK_NOTIFICATION_CARD_DETECT_CHANGED = 0x00000001ul,
    SDCARD_MONITOR_TASK_NOTIFICATION_INT_MAX             = 0x7FFFFFFFul,
};

enum _SDCARD_CARD_PRESENCE_STATE
{
    SDCARD_CARD_PRESENCE_STATE_INSERTED = 0x00000001ul,
    SDCARD_CARD_PRESENCE_STATE_REMOVED  = 0x00000002ul,
    SDCARD_CARD_PRESENCE_STATE_UNSTABLE = 0x00000003ul,
    SDCARD_CARD_PRESENCE_STATE_INT_MAX  = 0x7FFFFFFFul,
};

static void sdcard_cd_pin_exti_callback(void);
static uint32_t wait_for_stable_cd_pin_signal(const uint32_t timeout_ms);
static void sdcard_monitor_task(void *params);
static void sdcard_inserted_event_handler(bool *is_mounted);
static void sdcard_removed_event_handler(bool *is_mounted);
static int sdcard_mount(void);
static int sdcard_unmount(void);

int sdcard_monitor_init(void)
{
    h_sdcard_monitor_task = xTaskCreateStatic(sdcard_monitor_task,
                                              "SDCARD Monitor",
                                              SDCARD_MONITOR_TASK_STACKSIZE,
                                              NULL,
                                              SDCARD_MONITOR_TASK_PRIORITY,
                                              _sdcard_monitor_task_stack,
                                              &_sdcard_monitor_task_tcb);
    assert(h_sdcard_monitor_task);

    return sdcard_cd_pin_init(sdcard_cd_pin_exti_callback);
}

int sdcard_monitor_deinit(void)
{
    vTaskDelete(h_sdcard_monitor_task);
    h_sdcard_monitor_task = NULL;

    return sdcard_cd_pin_deinit();
}

/**
 * @brief Handles the event when the SD card is inserted.
 *
 * This function is called when the SD card insertion event is detected.
 * It attempts to mount the SD card if it is not already mounted.
 *
 * @param is_mounted Pointer to a boolean indicating whether the SD card is mounted.
 */
static void sdcard_inserted_event_handler(bool *is_mounted)
{
    if (false == *is_mounted)
    {
        if (0 == sdcard_mount())
        {
            *is_mounted = true;
            printf("    SD Card is mounted successfully.\r\n");
        }
    }
    else
    {
        printf("    The SD Card is not inserted properly.\r\n");
    }
}

/**
 * @brief Handles the event when the SD card is removed.
 *
 * This function is called when the SD card removal event is detected.
 * It attempts to unmount the SD card if it is currently mounted.
 *
 * @param is_mounted Pointer to a boolean indicating whether the SD card is mounted.
 */
static void sdcard_removed_event_handler(bool *is_mounted)
{
    if (true == *is_mounted)
    {
        if (0 == sdcard_unmount())
        {
            *is_mounted = false;
            printf("    SD Card is unmounted successfully.\r\n");
        }
    }
    else
    {
        printf("    The SD Card is not inserted properly.\r\n");
    }
}

/**
 * @brief Task responsible for monitoring the SD card presence.
 *
 * This task continuously monitors the state of the SD card using a notification mechanism.
 * Upon receiving the notification, it checks the current state of the card and invokes
 * the appropriate event handler function based on the detected state.
 *
 * @param params Pointer to task parameters (not used).
 */
static void sdcard_monitor_task(void *params)
{
    (void)params;

    bool is_mounted = false;
    uint32_t card_presence_state = wait_for_stable_cd_pin_signal(SDCARD_CD_PIN_DEBOUNCE_TIMEOUT_MS);

    if ((uint32_t)SDCARD_CARD_PRESENCE_STATE_INSERTED == card_presence_state)
    {
        sdcard_inserted_event_handler(&is_mounted);
    }

    for ( ;; )
    {
        uint32_t notification;
        xTaskNotifyWait(0, 0, &notification, portMAX_DELAY);

        if ((uint32_t)SDCARD_MONITOR_TASK_NOTIFICATION_CARD_DETECT_CHANGED == notification)
        {
            card_presence_state = wait_for_stable_cd_pin_signal(SDCARD_CD_PIN_DEBOUNCE_TIMEOUT_MS);

            switch (card_presence_state)
            {
                case (uint32_t)SDCARD_CARD_PRESENCE_STATE_INSERTED :
                {
                    sdcard_inserted_event_handler(&is_mounted);
                }
                break;

                case (uint32_t)SDCARD_CARD_PRESENCE_STATE_REMOVED :
                {
                    sdcard_removed_event_handler(&is_mounted);
                }
                break;

                case (uint32_t)SDCARD_CARD_PRESENCE_STATE_UNSTABLE :
                {
                    printf("    The memory card is not inserted properly.\r\n");
                }
                break;
            }
        }

        xTaskNotifyWait(0, ULONG_MAX, &notification, 0);
    }
}

/**
 * @brief Mounts the SD Card to the filesystem.
 *
 * @return 0 on success,
 * @return < 0 on failure.
 */
static int sdcard_mount(void)
{
    int err;

    mtd_sdcard.base.driver = &mtd_sdcard_driver;
    _fatfs_desc.dev = (mtd_dev_t *)&mtd_sdcard;

    err = vfs_mount(&_fatfs_sdcard_vfs_mount);
    printf("\r\n  sdcard_mount : %s\r\n", strerror(-err));

    return err;
}

/**
 * @brief Unmounts the SD Card from the filesystem.
 *
 * @return 0 on success,
 * @return < 0 on failure.
 */
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

/**
 * @brief Card-Detect pin EXTI Callback
 */
static void sdcard_cd_pin_exti_callback(void)
{
    BaseType_t higher_priority_task_woken = pdFALSE;
    xTaskNotifyFromISR(h_sdcard_monitor_task,
                       (uint32_t)SDCARD_MONITOR_TASK_NOTIFICATION_CARD_DETECT_CHANGED,
                       eSetBits,
                       &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

/**
 * @brief  Waits for a stable signal on the SD card detect pin.
 *
 * This function monitors the SD card detect pin until a stable signal is detected or a timeout occurs.
 * A stable signal indicates whether the SD card is inserted or removed.
 *
 * @param  timeout_ms The timeout value in milliseconds.
 *
 * @return The state of the SD card presence:
 *         - SDCARD_CARD_PRESENCE_STATE_INSERTED if the SD card is inserted.
 *         - SDCARD_CARD_PRESENCE_STATE_REMOVED if the SD card is removed.
 *         - SDCARD_CARD_PRESENCE_STATE_UNSTABLE if the SD card presence signal is unstable or a timeout occurs.
 */
static uint32_t wait_for_stable_cd_pin_signal(const uint32_t timeout_ms)
{
    uint32_t state = (uint32_t)SDCARD_CARD_PRESENCE_STATE_UNSTABLE;
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
        state = (uint32_t)SDCARD_CARD_PRESENCE_STATE_UNSTABLE;
    }
    else if (true == is_stable)
    {
        state = 0 == signal ? (uint32_t)SDCARD_CARD_PRESENCE_STATE_INSERTED : (uint32_t)SDCARD_CARD_PRESENCE_STATE_REMOVED;
    }

    return state;
}




