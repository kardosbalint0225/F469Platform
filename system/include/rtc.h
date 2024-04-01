/*
 * Original work Copyright (C) 2014 Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 * Modified work Copyright (C) 2024 Balint Kardos
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the root
 * directory of this software component or visit
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html for more details
 *
 * The original work can be found in the RIOT-OS 2024.01 branch:
 *     RIOT-OS repository: https://github.com/RIOT-OS/RIOT/tree/2024.01-branch
 *     rtc.h:              https://github.com/RIOT-OS/RIOT/blob/2024.01-branch/drivers/include/periph/rtc.h
 *
 * The original author of rtc.h is:
 *     Thomas Eichinger <thomas.eichinger@fu-berlin.de>
 */

/**
 * @ingroup     system_rtc
 * @brief       Low-level RTC (Real Time Clock) peripheral driver
 *
 * @note
 * The values used for setting and getting the time
 * conform to the `struct tm` specification.
 * Compare: http://pubs.opengroup.org/onlinepubs/7908799/xsh/time.h.html
 *
 * @{
 * @file        rtc.h
 * @brief       Low-level RTC peripheral driver interface definitions
 *
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

/**
 * @brief Get current RTC time with sub-second component.
 *        Requires the `periph_rtc_ms` feature.
 *
 * @param[out] time         Pointer to the struct to write the time to.
 * @param[out] ms           Pointer to a variable to hold the millisecond
 *                          component of the current RTC time.
 *
 * @return  0 for success
 * @return -1 an error occurred
 */
int rtc_get_time_ms(struct tm *time, uint16_t *ms);

#ifdef __cplusplus
}
#endif

#endif /* __RTC_H__ */
/** @} */
