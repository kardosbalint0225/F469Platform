/**
 * MIT License
 *
 * Copyright (c) 2024 Balint Kardos
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/**
 * @ingroup     core
 *
 * @file        runtime_stats_timer.h
 * @brief       Timer for Runtime Statistics
 */
#ifndef __RUN_TIME_STATS_TIMER_H__
#define __RUN_TIME_STATS_TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief Initializes dedicated Timer peripheral for generating the runtime statistics.
 *
 * This function initializes a timer peripheral (defined in FreeRTOSConfig.h)
 * to serve as the runtime statistics timer. The timer is configured
 * to generate interrupts periodically at 10 kHz fixed interval.
 *
 * @note  This function is called by the FreeRTOS kernel
 */
void runtime_stats_timer_init(void);

/**
 * @brief De-initializes the runtime statistics Timer.
 */
void runtime_stats_timer_deinit(void);

/**
 * @brief Gets the count of the runtime statistics timer.
 *
 * This function retrieves the count of the runtime statistics timer.
 *
 * @return the count of the runtime statistics timer.
 */
uint32_t runtime_stats_timer_get_count(void);

#ifdef __cplusplus
}
#endif
#endif /* __RUN_TIME_STATS_TIMER_H__ */

