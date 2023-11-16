/*
 * sdcard_mount.c
 *
 *  Created on: 2023. nov. 13.
 *      Author: Balint
 */
#include "sdcard_mount.h"
#include "sdcard_config.h"
#include "stm32f4xx_hal.h"

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

#define MNT_PATH  "/test"

static fatfs_desc_t fatfs;
static vfs_mount_t _test_vfs_mount = {
    .mount_point = MNT_PATH,
    .fs = &fatfs_file_system,
    .private_data = (void *)&fatfs,
};

mtd_sdcard_t mtd_sdcard_devs[1];
/* always default to first sdcard*/
static mtd_dev_t *mtd_sdcard = (mtd_dev_t*)&mtd_sdcard_devs[0];

TCHAR current_directory[512] = {'\0'};

typedef union {
    struct {
        uint16_t sec  : 5;
        uint16_t min  : 6;
        uint16_t hour : 5;
    };
    uint16_t w;
} fattime_t;

typedef union {
    struct {
        uint16_t day  : 5;
        uint16_t month: 4;
        uint16_t year : 7;
    };
    uint16_t w;
} fatdate_t;

EXTI_HandleTypeDef h_exti_sdcard_cd_pin;

static StackType_t sdcard_mount_task_stack[SDCARD_MOUNT_TASK_STACKSIZE];
static StaticTask_t sdcard_mount_task_tcb;
static TaskHandle_t h_sdcard_mount_task = NULL;

static bool is_mounted = false;

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

int sdcard_cd_pin_init(void);
int sdcard_cd_pin_deinit(void);
static void exti_sdcard_cd_pin_callback(void);
static void wait_for_stable_cd_pin_signal(const uint32_t timeout_ms, uint32_t *state);
static void sdcard_mount_task(void *params);

int sdcard_mount_init(void)
{
    h_sdcard_mount_task = xTaskCreateStatic(sdcard_mount_task,
                                            "SDCARD Mount",
                                            SDCARD_MOUNT_TASK_STACKSIZE,
                                            NULL,
                                            SDCARD_MOUNT_TASK_PRIORITY,
                                            sdcard_mount_task_stack,
                                            &sdcard_mount_task_tcb);
    assert_param(NULL != h_sdcard_mount_task);

    is_mounted = false;

    sdcard_cd_pin_init();

    return 0;
}

int sdcard_mount_deinit(void)
{
    vTaskDelete(h_sdcard_mount_task);
    h_sdcard_mount_task = NULL;

    sdcard_cd_pin_deinit();

    return 0;
}

static void sdcard_mount_task(void *params)
{
    (void)params;

    vTaskDelay(1000);

    uint32_t card_presence_state;
    wait_for_stable_cd_pin_signal(SDCARD_CD_PIN_DEBOUNCE_TIMEOUT_MS, &card_presence_state);
    if ((uint32_t)SDCARD_CARD_PRESENCE_STATE_INSERTED == card_presence_state)
    {
        //TODO: vfs_mount
        int err;

        mtd_sdcard_devs[0].base.driver = &mtd_sdcard_driver;
        err = mtd_init(&mtd_sdcard_devs[0].base);
        printf("mtd_init : %d\r\n", err);

        fatfs.dev = mtd_sdcard;

        err = vfs_mount(&_test_vfs_mount);
        printf("vfs_mount : %d\r\n", err);

        DIR dir;
        FILINFO fno;

        if (FR_OK == f_getcwd(current_directory, sizeof(current_directory)))
        {
            if (FR_OK == f_opendir(&dir, current_directory))
            {
                f_readdir(&dir, &fno);

                for (uint32_t i = 0; (i < 255) && (0 != fno.fname[0]); i++)
                {
//                    fattime_t fattime = {
//                        .w = fno.ftime,
//                    };
//
//                    fatdate_t fatdate = {
//                        .w = fno.fdate,
//                    };

//                    printf("%04d.%02d.%02d  %02d:%02d  %s  %16llu  %s\r\n",
//                           (fatdate.year+1980),
//                           fatdate.month,
//                           fatdate.day,
//                           fattime.hour,
//                           fattime.min,
//                           (fno.fattrib & AM_DIR) ? ("<DIR>") : ("     "),
//                           fno.fsize,
//                           fno.fname);
                    printf("%s\r\n", fno.fname);

                    f_readdir(&dir, &fno);
                }
            }

            f_closedir(&dir);
        }

        is_mounted = true;
    }

    BaseType_t ret;

    for ( ;; )
    {
        uint32_t notification;

        ret = xTaskNotifyWait(0, 0, &notification, portMAX_DELAY);
        assert_param(pdPASS == ret);

        if ((uint32_t)SDCARD_MOUNT_TASK_NOTIFICATION_CARD_DETECT_CHANGED == notification)
        {
            wait_for_stable_cd_pin_signal(SDCARD_CD_PIN_DEBOUNCE_TIMEOUT_MS, &card_presence_state);

            switch(card_presence_state)
            {
                case (uint32_t)SDCARD_CARD_PRESENCE_STATE_INSERTED :
                {
                    if (false == is_mounted)
                    {
                        //TODO: vfs_mount
                        printf("sdcard card is mounted\r\n");
                        is_mounted = true;
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
                        //TODO: vfs_unmount
                        printf("sdcard card is unmounted\r\n");
                        is_mounted = false;
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
                    assert_param(0);
                }
                break;
            }
        }

        xTaskNotifyWait(0, ULONG_MAX, &notification, 0);
    }
}

int sdcard_cd_pin_init(void)
{
    SDCARD_CD_PIN_GPIO_CLK_ENABLE();

    GPIO_InitTypeDef card_detect_pin_config = {
        .Pin = SDCARD_CD_PIN,
        .Mode = GPIO_MODE_IT_RISING_FALLING,
        .Pull = GPIO_PULLUP,
    };
    HAL_GPIO_Init(SDCARD_CD_PIN_GPIO_PORT, &card_detect_pin_config);

    EXTI_ConfigTypeDef exti_config = {
        .Line = SDCARD_CD_PIN_EXTI_LINE,
        .Mode = EXTI_MODE_INTERRUPT,
        .Trigger = EXTI_TRIGGER_RISING_FALLING,
        .GPIOSel = SDCARD_CD_PIN_EXTI_GPIO
    };

    HAL_StatusTypeDef ret;
    ret = HAL_EXTI_SetConfigLine(&h_exti_sdcard_cd_pin, &exti_config);
    assert_param(HAL_OK == ret);

    ret = HAL_EXTI_RegisterCallback(&h_exti_sdcard_cd_pin, HAL_EXTI_COMMON_CB_ID, exti_sdcard_cd_pin_callback);
    assert_param(HAL_OK == ret);

    HAL_NVIC_SetPriority(SDCARD_CD_PIN_EXTIx_IRQn, SDCARD_CD_PIN_EXTIx_IRQ_PRIORITY, 0U);
    HAL_NVIC_EnableIRQ(SDCARD_CD_PIN_EXTIx_IRQn);

    return 0;
}

int sdcard_cd_pin_deinit(void)
{
    HAL_StatusTypeDef ret;

    HAL_NVIC_DisableIRQ(SDCARD_CD_PIN_EXTIx_IRQn);

    ret = HAL_EXTI_ClearConfigLine(&h_exti_sdcard_cd_pin);
    assert_param(HAL_OK == ret);

    HAL_GPIO_DeInit(SDCARD_CD_PIN_GPIO_PORT, SDCARD_CD_PIN);

    return 0;
}

static void exti_sdcard_cd_pin_callback(void)
{
    BaseType_t higher_priority_task_woken = pdFALSE;
    BaseType_t ret = xTaskNotifyFromISR(h_sdcard_mount_task,
                                        (uint32_t)SDCARD_MOUNT_TASK_NOTIFICATION_CARD_DETECT_CHANGED,
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




