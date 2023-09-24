/**
 ******************************************************************************
 * @file    rtc.h
 * @brief   This file contains all the function prototypes for
 *          the rtc.c file
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
#ifndef __RTC_H__
#define __RTC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <time.h>
#include "rtc_utils.h"

#define _EPOCH_YEAR    1970U

enum _RTC_ERROR
{
    RTC_ERROR_REGISTER_MSPINIT_CB        = 0x00000001UL,
    RTC_ERROR_REGISTER_MSPDEINIT_CB      = 0x00000002UL,
    RTC_ERROR_RTC_INIT                   = 0x00000004UL,
    RTC_ERROR_RTC_SET_TIME               = 0x00000008UL,
    RTC_ERROR_RTC_SET_DATE               = 0x00000010UL,
    RTC_ERROR_RCC_PERIPHCLKCONFIG_LSE    = 0x00000020UL,
    RTC_ERROR_RTC_DEINIT                 = 0x00000040UL,
    RTC_ERROR_UNREGISTER_MSPINIT_CB      = 0x00000080UL,
    RTC_ERROR_UNREGISTER_MSPDEINIT_CB    = 0x00000100UL,
    RTC_ERROR_RCC_PERIPHCLKCONFIG_NO_CLK = 0x00000200UL,
    RTC_ERROR_RTC_GET_TIME               = 0x00000400UL,
    RTC_ERROR_RTC_GET_DATE               = 0x00000800UL,

    RTC_ERROR_UINT_MAX                   = 0xFFFFFFFFUL,
};


void     rtc_init(void);
void     rtc_deinit(void);
int      rtc_set_time(struct tm *time);
int      rtc_get_time(struct tm *time);
uint32_t rtc_get_error(void);

#ifdef __cplusplus
}
#endif

#endif /* __RTC_H__ */

