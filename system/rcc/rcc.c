/*
 * rcc.c
 *
 *  Created on: Feb 25, 2024
 *      Author: Balint
 */
#include "rcc.h"

HAL_StatusTypeDef rtc_clock_source_init(void)
{
    RCC_PeriphCLKInitTypeDef rtc_clock = {
        .PeriphClockSelection = RCC_PERIPHCLK_RTC,
        .RTCClockSelection = RCC_RTCCLKSOURCE_LSE,
    };
    return HAL_RCCEx_PeriphCLKConfig(&rtc_clock);
}

HAL_StatusTypeDef rtc_clock_source_deinit(void)
{
    RCC_PeriphCLKInitTypeDef rtc_clock = {
        .PeriphClockSelection = RCC_PERIPHCLK_RTC,
        .RTCClockSelection = RCC_RTCCLKSOURCE_NO_CLK,
    };
    return HAL_RCCEx_PeriphCLKConfig(&rtc_clock);
}

