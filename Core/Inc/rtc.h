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

void RTC_Init(void);
void RTC_Deinit(void);
void RTC_GetTime(uint8_t *hours, uint8_t *minutes, uint8_t *seconds);
void RTC_SetTime(uint8_t hours, uint8_t minutes, uint8_t seconds);
void RTC_GetDate(uint8_t *day, uint8_t *month, uint8_t *year, uint8_t *weekday);
void RTC_SetDate(uint8_t day, uint8_t month, uint8_t year);

#ifdef __cplusplus
}
#endif

#endif /* __RTC_H__ */

