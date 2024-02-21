#ifndef __RUN_TIME_STATS_TIMER_H__
#define __RUN_TIME_STATS_TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void     runtime_stats_timer_init(void);
void     runtime_stats_timer_deinit(void);
uint32_t runtime_stats_timer_get_count(void);

#ifdef __cplusplus
}
#endif
#endif /* __RUN_TIME_STATS_TIMER_H__ */

