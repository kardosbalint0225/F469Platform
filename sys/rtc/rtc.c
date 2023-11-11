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

/* struct tm counts years since 1900 but RTC has only two-digit year, hence the offset */
#define YEAR_OFFSET    (_EPOCH_YEAR - 1900)

RTC_HandleTypeDef hrtc;
static uint32_t rtc_error;
static SemaphoreHandle_t h_rtc_mutex = NULL;
static StaticSemaphore_t rtc_mutex_storage;

static void rtc_msp_init(RTC_HandleTypeDef * hrtc);
static void rtc_msp_deinit(RTC_HandleTypeDef * hrtc);
static void rtc_lock(void);
static void rtc_unlock(void);

void rtc_init(void)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    HAL_StatusTypeDef ret;

    rtc_error = 0UL;

    h_rtc_mutex = xSemaphoreCreateMutexStatic(&rtc_mutex_storage);

    hrtc.Instance = RTC;
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = 127;
    hrtc.Init.SynchPrediv = 255;
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

    ret = HAL_RTC_RegisterCallback(&hrtc, HAL_RTC_MSPINIT_CB_ID, rtc_msp_init);
    rtc_error |= (HAL_OK != ret) ? RTC_ERROR_REGISTER_MSPINIT_CB : 0UL;
    assert_param(0UL == rtc_error);

    ret = HAL_RTC_RegisterCallback(&hrtc, HAL_RTC_MSPDEINIT_CB_ID, rtc_msp_deinit);
    rtc_error |= (HAL_OK != ret) ? RTC_ERROR_REGISTER_MSPDEINIT_CB : 0UL;
    assert_param(0UL == rtc_error);

    ret = HAL_RTC_Init(&hrtc);
    rtc_error |= (HAL_OK != ret) ? RTC_ERROR_RTC_INIT : 0UL;
    assert_param(0UL == rtc_error);

    sTime.Hours = 12;
    sTime.Minutes = 0;
    sTime.Seconds = 0;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sTime.StoreOperation = RTC_STOREOPERATION_RESET;

    ret = HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    rtc_error |= (HAL_OK != ret) ? RTC_ERROR_RTC_SET_TIME : 0UL;
    assert_param(0UL == rtc_error);

    sDate.WeekDay = RTC_WEEKDAY_SATURDAY;
    sDate.Month = RTC_MONTH_JANUARY;
    sDate.Date = 1;
    sDate.Year = 22;

    ret = HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    rtc_error |= (HAL_OK != ret) ? RTC_ERROR_RTC_SET_DATE : 0UL;
    assert_param(0UL == rtc_error);
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
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    HAL_StatusTypeDef ret;

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;

    ret = HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
    rtc_error |= (HAL_OK != ret) ? RTC_ERROR_RCC_PERIPHCLKCONFIG_LSE : 0UL;
    assert_param(0UL == rtc_error);

    __HAL_RCC_RTC_ENABLE();
}

void rtc_deinit(void)
{
    HAL_StatusTypeDef ret;

    ret = HAL_RTC_DeInit(&hrtc);
    rtc_error |= (HAL_OK != ret) ? RTC_ERROR_RTC_DEINIT : 0UL;
    assert_param(0UL == rtc_error);

    ret = HAL_RTC_UnRegisterCallback(&hrtc, HAL_RTC_MSPINIT_CB_ID);
    rtc_error |= (HAL_OK != ret) ? RTC_ERROR_UNREGISTER_MSPINIT_CB : 0UL;
    assert_param(0UL == rtc_error);

    ret = HAL_RTC_UnRegisterCallback(&hrtc, HAL_RTC_MSPDEINIT_CB_ID);
    rtc_error |= (HAL_OK != ret) ? RTC_ERROR_UNREGISTER_MSPDEINIT_CB : 0UL;
    assert_param(0UL == rtc_error);

    vSemaphoreDelete(h_rtc_mutex);
    h_rtc_mutex = NULL;
}

/**
 * @brief  Deinitializes the RTC MSP
 * @param  hrtc pointer to the RTC_HandleTypeDef structure
 * @retval None
 * @note   This function is called by the HAL library when
 *         rtc_deinit functions is called
 */
static void rtc_msp_deinit(RTC_HandleTypeDef * hrtc)
{
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
    HAL_StatusTypeDef ret;

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_NO_CLK;

    ret = HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
    rtc_error |= (HAL_OK != ret) ? RTC_ERROR_RCC_PERIPHCLKCONFIG_NO_CLK : 0UL;
    assert_param(0UL == rtc_error);

    __HAL_RCC_RTC_DISABLE();
}

uint32_t rtc_get_error(void)
{
    return rtc_error;
}

int rtc_set_time(struct tm *time)
{
    rtc_tm_normalize(time);

    RTC_TimeTypeDef stime = {0};
    RTC_DateTypeDef sdate = {0};
    HAL_StatusTypeDef ret;

    rtc_lock();

    ret = HAL_RTC_GetTime(&hrtc, &stime, RTC_FORMAT_BIN);
    rtc_error |= (HAL_OK != ret) ? RTC_ERROR_RTC_GET_TIME : 0UL;
    assert_param(0UL == rtc_error);

    ret = HAL_RTC_GetDate(&hrtc, &sdate, RTC_FORMAT_BIN);
    rtc_error |= (HAL_OK != ret) ? RTC_ERROR_RTC_GET_DATE : 0UL;
    assert_param(0UL == rtc_error);

    sdate.Year = time->tm_year - YEAR_OFFSET;
    sdate.Month = time->tm_mon + 1;
    sdate.Date = time->tm_mday;

    ret = HAL_RTC_SetDate(&hrtc, &sdate, RTC_FORMAT_BIN);
    rtc_error |= (HAL_OK != ret) ? RTC_ERROR_RTC_SET_DATE : 0UL;
    assert_param(0UL == rtc_error);

    stime.Hours = time->tm_hour;
    stime.Minutes = time->tm_min;
    stime.Seconds = time->tm_sec;

    ret = HAL_RTC_SetTime(&hrtc, &stime, RTC_FORMAT_BIN);
    rtc_error |= (HAL_OK != ret) ? RTC_ERROR_RTC_SET_TIME : 0UL;
    assert_param(0UL == rtc_error);

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
    rtc_error |= (HAL_OK != ret) ? RTC_ERROR_RTC_GET_TIME : 0UL;
    assert_param(0UL == rtc_error);

    ret = HAL_RTC_GetDate(&hrtc, &sdate, RTC_FORMAT_BIN);
    rtc_error |= (HAL_OK != ret) ? RTC_ERROR_RTC_GET_DATE : 0UL;
    assert_param(0UL == rtc_error);

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
    BaseType_t ret = xSemaphoreTake(h_rtc_mutex, portMAX_DELAY);
    assert_param(pdPASS == ret);
}

/**
 * @brief  Unlocks the RTC mutex
 */
static void rtc_unlock(void)
{
    BaseType_t ret = xSemaphoreGive(h_rtc_mutex);
    assert_param(pdPASS == ret);
}






