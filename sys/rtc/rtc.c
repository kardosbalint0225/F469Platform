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
#include "stm32f4xx_hal.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "hal_errno.h"
#include <errno.h>

/* struct tm counts years since 1900 but RTC has only two-digit year, hence the offset */
#define YEAR_OFFSET    (_EPOCH_YEAR - 1900)

RTC_HandleTypeDef hrtc;
static SemaphoreHandle_t _rtc_mutex = NULL;
static StaticSemaphore_t _rtc_mutex_storage;
static HAL_StatusTypeDef _error = HAL_OK; /**< for msp init/deinit error handling */

static void rtc_msp_init(RTC_HandleTypeDef *hrtc);
static void rtc_msp_deinit(RTC_HandleTypeDef *hrtc);
static void rtc_lock(void);
static void rtc_unlock(void);


int rtc_init(void)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    HAL_StatusTypeDef ret;

    _error = HAL_OK;
    _rtc_mutex = xSemaphoreCreateMutexStatic(&_rtc_mutex_storage);
    if (NULL == _rtc_mutex)
    {
        return -ENOMEM;
    }

    hrtc.Instance = RTC;
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = 127;
    hrtc.Init.SynchPrediv = 255;
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

    ret = HAL_RTC_RegisterCallback(&hrtc, HAL_RTC_MSPINIT_CB_ID, rtc_msp_init);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_RTC_RegisterCallback(&hrtc, HAL_RTC_MSPDEINIT_CB_ID, rtc_msp_deinit);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_RTC_Init(&hrtc);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    if (HAL_OK != _error)
    {
        return hal_statustypedef_to_errno(_error);
    }

    sTime.Hours = 12;
    sTime.Minutes = 0;
    sTime.Seconds = 0;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    ret = HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    sDate.WeekDay = RTC_WEEKDAY_SATURDAY;
    sDate.Month = RTC_MONTH_JANUARY;
    sDate.Date = 1;
    sDate.Year = 22;

    ret = HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

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
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {
        .PeriphClockSelection = RCC_PERIPHCLK_RTC,
        .RTCClockSelection = RCC_RTCCLKSOURCE_LSE,
    };

    _error = HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
    __HAL_RCC_RTC_ENABLE();
}

int rtc_deinit(void)
{
    HAL_StatusTypeDef ret;

    ret = HAL_RTC_DeInit(&hrtc);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    if (HAL_OK != _error)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_RTC_UnRegisterCallback(&hrtc, HAL_RTC_MSPINIT_CB_ID);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_RTC_UnRegisterCallback(&hrtc, HAL_RTC_MSPDEINIT_CB_ID);
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
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {
        .PeriphClockSelection = RCC_PERIPHCLK_RTC,
        .RTCClockSelection = RCC_RTCCLKSOURCE_NO_CLK,
    };

    _error = HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
    __HAL_RCC_RTC_DISABLE();
}


int rtc_set_time(struct tm *time)
{
    rtc_tm_normalize(time);

    RTC_TimeTypeDef stime = {0};
    RTC_DateTypeDef sdate = {0};
    HAL_StatusTypeDef ret;

    rtc_lock();

    ret = HAL_RTC_GetTime(&hrtc, &stime, RTC_FORMAT_BIN);
    if (HAL_OK != ret)
    {
        rtc_unlock();
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_RTC_GetDate(&hrtc, &sdate, RTC_FORMAT_BIN);
    if (HAL_OK != ret)
    {
        rtc_unlock();
        return hal_statustypedef_to_errno(ret);
    }

    sdate.Year = time->tm_year - YEAR_OFFSET;
    sdate.Month = time->tm_mon + 1;
    sdate.Date = time->tm_mday;

    ret = HAL_RTC_SetDate(&hrtc, &sdate, RTC_FORMAT_BIN);
    if (HAL_OK != ret)
    {
        rtc_unlock();
        return hal_statustypedef_to_errno(ret);
    }

    stime.Hours = time->tm_hour;
    stime.Minutes = time->tm_min;
    stime.Seconds = time->tm_sec;

    ret = HAL_RTC_SetTime(&hrtc, &stime, RTC_FORMAT_BIN);
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
    RTC_TimeTypeDef stime = {0};
    RTC_DateTypeDef sdate = {0};
    HAL_StatusTypeDef ret;

    rtc_lock();

    ret = HAL_RTC_GetTime(&hrtc, &stime, RTC_FORMAT_BIN);
    if (HAL_OK != ret)
    {
        rtc_unlock();
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_RTC_GetDate(&hrtc, &sdate, RTC_FORMAT_BIN);
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

    return 0;
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






