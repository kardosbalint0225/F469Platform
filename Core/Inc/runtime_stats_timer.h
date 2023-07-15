/*
 * run_time_stats_timer.h
 *
 *  Created on: 2023. jul. 12.
 *      Author: Balint
 */

#ifndef _RUN_TIME_STATS_TIMER_H_
#define _RUN_TIME_STATS_TIMER_H_

#include <stdint.h>

enum _RUNTIME_STATS_TIMER_ERROR
{
    RUNTIME_STATS_TIMER_ERROR_TIM_BASE_INIT                = 0x00000001UL,
	RUNTIME_STATS_TIMER_ERROR_REGISTER_PERIOD_ELAPSED_CB   = 0x00000002UL,
	RUNTIME_STATS_TIMER_ERROR_TIM_BASE_START_IT            = 0x00000004UL,
	RUNTIME_STATS_TIMER_ERROR_TIM_BASE_STOP_IT             = 0x00000008UL,
	RUNTIME_STATS_TIMER_ERROR_UNREGISTER_PERIOD_ELAPSED_CB = 0x00000010UL,
	RUNTIME_STATS_TIMER_ERROR_TIM_BASE_DEINIT              = 0x00000020UL,
};

extern volatile uint32_t runtime_stats_timer;

void runtime_stats_timer_init(void);
void runtime_stats_timer_deinit(void);

#endif /* _RUN_TIME_STATS_TIMER_H_ */
