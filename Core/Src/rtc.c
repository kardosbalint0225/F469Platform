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

static void RTC_MspInit(RTC_HandleTypeDef* hrtc);
static void RTC_MspDeInit(RTC_HandleTypeDef* hrtc);

/**
  * @brief  Initializes the RTC peripheral
  * @param	None
  * @retval None
  */
void RTC_Init(void)
{
	RTC_TimeTypeDef sTime    = {0};
	RTC_DateTypeDef sDate    = {0};
	HAL_StatusTypeDef ret;

	/** Initialize RTC Only	*/
	hrtc.Instance            = RTC;
	hrtc.Init.HourFormat     = RTC_HOURFORMAT_24;
	hrtc.Init.AsynchPrediv   = 127;
	hrtc.Init.SynchPrediv    = 255;
	hrtc.Init.OutPut         = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType     = RTC_OUTPUT_TYPE_OPENDRAIN;

	ret = HAL_RTC_RegisterCallback(&hrtc, HAL_RTC_MSPINIT_CB_ID, RTC_MspInit);
	assert_param(HAL_OK == ret);

	ret = HAL_RTC_RegisterCallback(&hrtc, HAL_RTC_MSPDEINIT_CB_ID, RTC_MspDeInit);
	assert_param(HAL_OK == ret);

	ret = HAL_RTC_Init(&hrtc);
	assert_param(HAL_OK == ret);

	/** Initialize RTC and set the Time and Date */
	sTime.Hours              = 12;
	sTime.Minutes            = 0;
	sTime.Seconds            = 0;
	sTime.DayLightSaving     = RTC_DAYLIGHTSAVING_NONE;
	sTime.StoreOperation     = RTC_STOREOPERATION_RESET;

	ret = HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	assert_param(HAL_OK == ret);

	sDate.WeekDay            = RTC_WEEKDAY_SATURDAY;
	sDate.Month              = RTC_MONTH_JANUARY;
	sDate.Date               = 1;
	sDate.Year               = 22;

	ret = HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	assert_param(HAL_OK == ret);
}

/**
  * @brief  Initializes the RTC peripheral low-level
  * @param	hrtc pointer to the RTC_HandleTypeDef structure
  * @retval None
  * @note   The RTC peripheral uses LSI as its clock source
  * @note 	This function is called by the HAL library when
  * 		RTC_Init functions is called
  */
static void RTC_MspInit(RTC_HandleTypeDef* hrtc)
{
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
	HAL_StatusTypeDef ret;

	/** Initializes the peripherals clock */
	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
	PeriphClkInitStruct.RTCClockSelection    = RCC_RTCCLKSOURCE_LSI;

	ret = HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
	assert_param(HAL_OK == ret);

	/* RTC clock enable */
	__HAL_RCC_RTC_ENABLE();
}

/**
  * @brief  Deinitializes the RTC peripheral
  * @param	None
  * @retval None
  */
void RTC_Deinit(void)
{
	HAL_StatusTypeDef ret;

	ret = HAL_RTC_DeInit(&hrtc);
	assert_param(HAL_OK == ret);

	ret = HAL_RTC_UnRegisterCallback(&hrtc, HAL_RTC_MSPINIT_CB_ID);
	assert_param(HAL_OK == ret);

	ret = HAL_RTC_UnRegisterCallback(&hrtc, HAL_RTC_MSPDEINIT_CB_ID);
	assert_param(HAL_OK == ret);
}

/**
  * @brief  Deinitializes the RTC peripheral low-level
  * @param	hrtc pointer to the RTC_HandleTypeDef structure
  * @retval None
  * @note   This function is called by the HAL library when
  * 		RTC_Deinit functions is called
  */
static void RTC_MspDeInit(RTC_HandleTypeDef* hrtc)
{
	RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
	HAL_StatusTypeDef ret;

	PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
	PeriphClkInitStruct.RTCClockSelection    = RCC_RTCCLKSOURCE_NO_CLK;

	ret = HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
	assert_param(HAL_OK == ret);

	/* Peripheral clock disable */
	__HAL_RCC_RTC_DISABLE();
}

/**
  * @brief  Gets the current time from the RTC peripheral
  * @param	hours points where the hours value can be stored
  * @param  minutes points where the minutes value can be stored
  * @param  seconds points where the seconds value can be stored
  * @retval None
  */
void RTC_GetTime(uint8_t *hours, uint8_t *minutes, uint8_t *seconds)
{
	RTC_TimeTypeDef stime = {0};
	RTC_DateTypeDef sdate = {0};
	HAL_StatusTypeDef ret;

	ret = HAL_RTC_GetTime(&hrtc, &stime, RTC_FORMAT_BIN);
	assert_param(HAL_OK == ret);

	ret = HAL_RTC_GetDate(&hrtc, &sdate, RTC_FORMAT_BIN);
	assert_param(HAL_OK == ret);

	*hours   = stime.Hours;
	*minutes = stime.Minutes;
	*seconds = stime.Seconds;
}

/**
  * @brief  Gets the current date from the RTC peripheral
  * @param	day points where the day value can be stored
  * @param  month points where the month value can be stored
  * @param  year points where the year value can be stored
  * @retval None
  */
void RTC_GetDate(uint8_t *day, uint8_t *month, uint8_t *year, uint8_t *weekday)
{
	RTC_TimeTypeDef stime = {0};
	RTC_DateTypeDef sdate = {0};
	HAL_StatusTypeDef ret;

	ret = HAL_RTC_GetTime(&hrtc, &stime, RTC_FORMAT_BIN);
	assert_param(HAL_OK == ret);

	ret = HAL_RTC_GetDate(&hrtc, &sdate, RTC_FORMAT_BIN);
	assert_param(HAL_OK == ret);

	*day   = sdate.Date;
	*month = sdate.Month;
	*year  = sdate.Year;
	*weekday = sdate.WeekDay;
}

/**
  * @brief  Sets the time to the RTC peripheral
  * @param	hours value to be set
  * @param  minutes value to be set
  * @param  seconds value to be set
  * @retval None
  */
void RTC_SetTime(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
	RTC_TimeTypeDef stime = {0};
	RTC_DateTypeDef sdate = {0};
	HAL_StatusTypeDef ret;

	ret = HAL_RTC_GetTime(&hrtc, &stime, RTC_FORMAT_BIN);
	assert_param(HAL_OK == ret);

	ret = HAL_RTC_GetDate(&hrtc, &sdate, RTC_FORMAT_BIN);
	assert_param(HAL_OK == ret);

	stime.Hours    = hours;
	stime.Minutes  = minutes;
	stime.Seconds  = seconds;

	ret = HAL_RTC_SetTime(&hrtc, &stime, RTC_FORMAT_BIN);
	assert_param(HAL_OK == ret);
}

/**
  * @brief  Sets the date to the RTC peripheral
  * @param	day value to be set
  * @param  month value to be set
  * @param  year value to be set
  * @retval None
  */
void RTC_SetDate(uint8_t day, uint8_t month, uint8_t year)
{
	RTC_TimeTypeDef stime = {0};
	RTC_DateTypeDef sdate = {0};
	HAL_StatusTypeDef ret;

	ret = HAL_RTC_GetTime(&hrtc, &stime, RTC_FORMAT_BIN);
	assert_param(HAL_OK == ret);

	ret = HAL_RTC_GetDate(&hrtc, &sdate, RTC_FORMAT_BIN);
	assert_param(HAL_OK == ret);

	sdate.Date   = day;
	sdate.Month  = month;
	sdate.Year   = year;

	ret = HAL_RTC_SetDate(&hrtc, &sdate, RTC_FORMAT_BIN);
	assert_param(HAL_OK == ret);
}



