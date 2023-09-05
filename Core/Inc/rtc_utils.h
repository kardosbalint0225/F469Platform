/*
 * rtc_utils.h
 *
 *  Created on: 2023. aug. 31.
 *      Author: Balint
 */

#ifndef _RTC_UTILS_H_
#define _RTC_UTILS_H_

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

void rtc_tm_normalize(struct tm *t);
uint32_t rtc_mktime(struct tm *t);
int rtc_tm_compare(const struct tm *a, const struct tm *b);
bool rtc_tm_valid(const struct tm *t);

#endif /* _RTC_UTILS_H_ */
