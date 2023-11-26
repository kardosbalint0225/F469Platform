/*
 * run_time_stats_timer.h
 *
 *  Created on: 2023. jul. 12.
 *      Author: Balint
 */

#ifndef _RUN_TIME_STATS_TIMER_H_
#define _RUN_TIME_STATS_TIMER_H_

#include <stdint.h>

void     runtime_stats_timer_init(void);
void     runtime_stats_timer_deinit(void);
uint32_t runtime_stats_timer_get_count(void);

#endif /* _RUN_TIME_STATS_TIMER_H_ */

