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

/**
 * @brief Initialize RTC module
 *
 * @return  0 for success
 * @return < 0 an error occurred
 */
int rtc_init(void);

/**
 * @brief De-initialize RTC module

 * @return  0 for success
 * @return < 0 an error occurred
 */
int rtc_deinit(void);

/**
 * @brief Set RTC to given time.
 *
 * @param[in] time          Pointer to the struct holding the time to set.
 *
 * @return  0 for success
 * @return < 0 an error occurred
 */
int rtc_set_time(struct tm *time);

/**
 * @brief Get current RTC time.
 *
 * @param[out] time         Pointer to the struct to write the time to.
 *
 * @return  0 for success
 * @return < 0 an error occurred
 */
int rtc_get_time(struct tm *time);


#ifdef __cplusplus
}
#endif

#endif /* __RTC_H__ */

