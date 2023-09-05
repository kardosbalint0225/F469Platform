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
void     rtc_get_time(uint8_t *hours, uint8_t *minutes, uint8_t *seconds, uint32_t *subseconds);
void     rtc_set_time(uint8_t hours, uint8_t minutes, uint8_t seconds);
void     rtc_get_date(uint8_t *day, uint8_t *month, uint32_t *year, uint8_t *weekday);
void     rtc_set_date(uint8_t day, uint8_t month, uint32_t year);
uint32_t rtc_get_error(void);

#ifdef __cplusplus
}
#endif

#endif /* __RTC_H__ */

