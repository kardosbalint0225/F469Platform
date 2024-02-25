/**
 ******************************************************************************
 * @file    rtc.c
 * @brief   This file provides code for the configuration
 *          of the RTC instances.
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
#include "rtc.h"
#include "rcc.h"
#include "stm32f4xx_hal.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "hal_errno.h"
#include <errno.h>

/* struct tm counts years since 1900 but RTC has only two-digit year, hence the offset */
#define YEAR_OFFSET    (_EPOCH_YEAR - 1900)

RTC_HandleTypeDef h_rtc;
static SemaphoreHandle_t _rtc_mutex = NULL;
static StaticSemaphore_t _rtc_mutex_storage;
static HAL_StatusTypeDef _error = HAL_OK; /**< for msp init/deinit error handling */
static volatile uint32_t _tick_count_previous = 0ul;

static void rtc_msp_init(RTC_HandleTypeDef *hrtc);
static void rtc_msp_deinit(RTC_HandleTypeDef *hrtc);
static void rtc_wakeuptimer_event_callback(RTC_HandleTypeDef *hrtc);
static void rtc_lock(void);
static void rtc_unlock(void);


int rtc_init(void)
{
    HAL_StatusTypeDef ret;

    _error = HAL_OK;
    _rtc_mutex = xSemaphoreCreateMutexStatic(&_rtc_mutex_storage);
    if (NULL == _rtc_mutex)
    {
        return -ENOMEM;
    }

    h_rtc.Instance = RTC;
    h_rtc.Init.HourFormat = RTC_HOURFORMAT_24;
    h_rtc.Init.AsynchPrediv = 127;
    h_rtc.Init.SynchPrediv = 255;
    h_rtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    h_rtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    h_rtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

    ret = HAL_RTC_RegisterCallback(&h_rtc, HAL_RTC_MSPINIT_CB_ID, rtc_msp_init);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_RTC_RegisterCallback(&h_rtc, HAL_RTC_MSPDEINIT_CB_ID, rtc_msp_deinit);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_RTC_Init(&h_rtc);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    if (HAL_OK != _error)
    {
        return hal_statustypedef_to_errno(_error);
    }

    ret = HAL_RTC_RegisterCallback(&h_rtc, HAL_RTC_WAKEUPTIMER_EVENT_CB_ID, rtc_wakeuptimer_event_callback);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    struct tm inittime = {
        .tm_year = 2024 - 1900,
        .tm_mon = 1 - 1,
        .tm_mday = 1,
        .tm_hour = 12,
        .tm_min = 0,
        .tm_sec = 0,
    };

    if (0 != rtc_set_time(&inittime))
    {
        return -EIO;
    }

    ret = HAL_RTCEx_SetWakeUpTimer_IT(&h_rtc, 0x7FF, RTC_WAKEUPCLOCK_RTCCLK_DIV16);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    HAL_NVIC_SetPriority(RTC_WKUP_IRQn, 15, 0);
    HAL_NVIC_EnableIRQ(RTC_WKUP_IRQn);

    return 0;
}

/**
 * @brief  Initializes the RTC MSP
 * @param  hrtc pointer to the RTC_HandleTypeDef structure
 * @retval None
 * @note   The RTC peripheral uses LSE as its clock source
 * @note   This function is called by the HAL library when
 *         rtc_init functions is called
 */
static void rtc_msp_init(RTC_HandleTypeDef * hrtc)
{
    _error = rtc_clock_source_init();
    __HAL_RCC_RTC_ENABLE();
}

int rtc_deinit(void)
{
    HAL_StatusTypeDef ret;

    ret = HAL_RTCEx_DeactivateWakeUpTimer(&h_rtc);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_RTC_UnRegisterCallback(&h_rtc, HAL_RTC_WAKEUPTIMER_EVENT_CB_ID);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_RTC_DeInit(&h_rtc);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    if (HAL_OK != _error)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_RTC_UnRegisterCallback(&h_rtc, HAL_RTC_MSPINIT_CB_ID);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_RTC_UnRegisterCallback(&h_rtc, HAL_RTC_MSPDEINIT_CB_ID);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    vSemaphoreDelete(_rtc_mutex);
    _rtc_mutex = NULL;

    return 0;
}

/**
 * @brief  Deinitializes the RTC MSP
 * @param  hrtc pointer to the RTC_HandleTypeDef structure
 * @retval None
 * @note   This function is called by the HAL library when
 *         rtc_deinit functions is called
 */
static void rtc_msp_deinit(RTC_HandleTypeDef *hrtc)
{
    _error = rtc_clock_source_deinit();
    __HAL_RCC_RTC_DISABLE();
}


int rtc_set_time(struct tm *time)
{
    rtc_tm_normalize(time);

    RTC_TimeTypeDef stime = {0};
    RTC_DateTypeDef sdate = {0};
    HAL_StatusTypeDef ret;

    rtc_lock();

    ret = HAL_RTC_GetTime(&h_rtc, &stime, RTC_FORMAT_BIN);
    if (HAL_OK != ret)
    {
        rtc_unlock();
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_RTC_GetDate(&h_rtc, &sdate, RTC_FORMAT_BIN);
    if (HAL_OK != ret)
    {
        rtc_unlock();
        return hal_statustypedef_to_errno(ret);
    }

    sdate.Year = time->tm_year - YEAR_OFFSET;
    sdate.Month = time->tm_mon + 1;
    sdate.Date = time->tm_mday;

    ret = HAL_RTC_SetDate(&h_rtc, &sdate, RTC_FORMAT_BIN);
    if (HAL_OK != ret)
    {
        rtc_unlock();
        return hal_statustypedef_to_errno(ret);
    }

    stime.Hours = time->tm_hour;
    stime.Minutes = time->tm_min;
    stime.Seconds = time->tm_sec;

    ret = HAL_RTC_SetTime(&h_rtc, &stime, RTC_FORMAT_BIN);
    if (HAL_OK != ret)
    {
        rtc_unlock();
        return hal_statustypedef_to_errno(ret);
    }

    rtc_unlock();

    return 0;
}

int rtc_get_time(struct tm *time)
{
    return rtc_get_time_ms(time, NULL);
}

int rtc_get_time_ms(struct tm *time, uint16_t *ms)
{
    if (NULL == time)
    {
        return -1;
    }

    RTC_TimeTypeDef stime = {0};
    RTC_DateTypeDef sdate = {0};
    HAL_StatusTypeDef ret;

    rtc_lock();

    ret = HAL_RTC_GetTime(&h_rtc, &stime, RTC_FORMAT_BIN);
    if (HAL_OK != ret)
    {
        rtc_unlock();
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_RTC_GetDate(&h_rtc, &sdate, RTC_FORMAT_BIN);
    if (HAL_OK != ret)
    {
        rtc_unlock();
        return hal_statustypedef_to_errno(ret);
    }

    rtc_unlock();

    time->tm_year = sdate.Year + YEAR_OFFSET;
    time->tm_mon = sdate.Month - 1;
    time->tm_mday = sdate.Date;
    time->tm_hour = stime.Hours;
    time->tm_min = stime.Minutes;
    time->tm_sec = stime.Seconds;

    const uint32_t resolution_ms = (uint32_t)(1000ul / (uint32_t)configTICK_RATE_HZ);
    uint32_t tick_count_current = (uint32_t)xTaskGetTickCount();

    if (ms != NULL)
    {
        if (tick_count_current > _tick_count_previous)
        {
            *ms = (uint16_t)((tick_count_current - _tick_count_previous) * resolution_ms);
        }
        else
        {
            *ms = (uint16_t)((_tick_count_previous - tick_count_current) * resolution_ms);
        }
    }

    return 0;
}

static void rtc_wakeuptimer_event_callback(RTC_HandleTypeDef *hrtc)
{
    _tick_count_previous = (uint32_t)xTaskGetTickCountFromISR();
}

/**
 * @brief  Locks the RTC mutex
 */
static void rtc_lock(void)
{
    xSemaphoreTake(_rtc_mutex, portMAX_DELAY);
}

/**
 * @brief  Unlocks the RTC mutex
 */
static void rtc_unlock(void)
{
    xSemaphoreGive(_rtc_mutex);
}






