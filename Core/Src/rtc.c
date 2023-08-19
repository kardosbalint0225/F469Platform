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

RTC_HandleTypeDef hrtc;

static void rtc_msp_init(RTC_HandleTypeDef * hrtc);
static void rtc_msp_deinit(RTC_HandleTypeDef * hrtc);

static uint32_t rtc_error;
//TODO: weekday handling
/**
 * @brief  Initializes the RTC peripheral
 * @param  None
 * @retval None
 */
void rtc_init(void)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    HAL_StatusTypeDef ret;

    rtc_error = 0UL;

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

/**
 * @brief  Deinitializes the RTC peripheral
 * @param  None
 * @retval None
 */
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

/**
 * @brief  Gets the current error state of the RTC
 * @param  None
 * @retval 0 if no error occured
 *         positive value indicates error where each bit
 *         corresponds to a specific error defined in _RTC_ERROR
 * @note   -
 */
uint32_t rtc_get_error(void)
{
    return rtc_error;
}

/**
 * @brief  Gets the current time from the RTC peripheral
 * @param  hours points where the hours value can be stored
 * @param  minutes points where the minutes value can be stored
 * @param  seconds points where the seconds value can be stored
 * @retval None
 */
void rtc_get_time(uint8_t *hours, uint8_t *minutes, uint8_t *seconds)
{
    RTC_TimeTypeDef stime = {0};
    RTC_DateTypeDef sdate = {0};
    HAL_StatusTypeDef ret;

    ret = HAL_RTC_GetTime(&hrtc, &stime, RTC_FORMAT_BIN);
    rtc_error |= (HAL_OK != ret) ? RTC_ERROR_RTC_GET_TIME : 0UL;
    assert_param(0UL == rtc_error);

    ret = HAL_RTC_GetDate(&hrtc, &sdate, RTC_FORMAT_BIN);
    rtc_error |= (HAL_OK != ret) ? RTC_ERROR_RTC_GET_DATE : 0UL;
    assert_param(0UL == rtc_error);

    *hours = stime.Hours;
    *minutes = stime.Minutes;
    *seconds = stime.Seconds;
}

/**
 * @brief  Gets the current date from the RTC peripheral
 * @param  day points where the day value can be stored
 * @param  month points where the month value can be stored
 * @param  year points where the year value can be stored
 * @retval None
 */
void rtc_get_date(uint8_t *day, uint8_t *month, uint8_t *year, uint8_t *weekday)
{
    RTC_TimeTypeDef stime = {0};
    RTC_DateTypeDef sdate = {0};
    HAL_StatusTypeDef ret;

    ret = HAL_RTC_GetTime(&hrtc, &stime, RTC_FORMAT_BIN);
    rtc_error |= (HAL_OK != ret) ? RTC_ERROR_RTC_GET_TIME : 0UL;
    assert_param(0UL == rtc_error);

    ret = HAL_RTC_GetDate(&hrtc, &sdate, RTC_FORMAT_BIN);
    rtc_error |= (HAL_OK != ret) ? RTC_ERROR_RTC_GET_DATE : 0UL;
    assert_param(0UL == rtc_error);

    *day = sdate.Date;
    *month = sdate.Month;
    *year = sdate.Year;
    *weekday = sdate.WeekDay;
}

/**
 * @brief  Sets the time to the RTC peripheral
 * @param  hours value to be set
 * @param  minutes value to be set
 * @param  seconds value to be set
 * @retval None
 */
void rtc_set_time(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    RTC_TimeTypeDef stime = {0};
    RTC_DateTypeDef sdate = {0};
    HAL_StatusTypeDef ret;

    ret = HAL_RTC_GetTime(&hrtc, &stime, RTC_FORMAT_BIN);
    rtc_error |= (HAL_OK != ret) ? RTC_ERROR_RTC_GET_TIME : 0UL;
    assert_param(0UL == rtc_error);

    ret = HAL_RTC_GetDate(&hrtc, &sdate, RTC_FORMAT_BIN);
    rtc_error |= (HAL_OK != ret) ? RTC_ERROR_RTC_GET_DATE : 0UL;
    assert_param(0UL == rtc_error);

    stime.Hours = hours;
    stime.Minutes = minutes;
    stime.Seconds = seconds;

    ret = HAL_RTC_SetTime(&hrtc, &stime, RTC_FORMAT_BIN);
    rtc_error |= (HAL_OK != ret) ? RTC_ERROR_RTC_SET_TIME : 0UL;
    assert_param(0UL == rtc_error);
}

/**
 * @brief  Sets the date to the RTC peripheral
 * @param  day value to be set
 * @param  month value to be set
 * @param  year value to be set
 * @retval None
 */
void rtc_set_date(uint8_t day, uint8_t month, uint8_t year)
{
    RTC_TimeTypeDef stime = {0};
    RTC_DateTypeDef sdate = {0};
    HAL_StatusTypeDef ret;

    ret = HAL_RTC_GetTime(&hrtc, &stime, RTC_FORMAT_BIN);
    rtc_error |= (HAL_OK != ret) ? RTC_ERROR_RTC_GET_TIME : 0UL;
    assert_param(0UL == rtc_error);

    ret = HAL_RTC_GetDate(&hrtc, &sdate, RTC_FORMAT_BIN);
    rtc_error |= (HAL_OK != ret) ? RTC_ERROR_RTC_GET_DATE : 0UL;
    assert_param(0UL == rtc_error);

    sdate.Date = day;
    sdate.Month = month;
    sdate.Year = year;

    ret = HAL_RTC_SetDate(&hrtc, &sdate, RTC_FORMAT_BIN);
    rtc_error |= (HAL_OK != ret) ? RTC_ERROR_RTC_SET_DATE : 0UL;
    assert_param(0UL == rtc_error);
}

