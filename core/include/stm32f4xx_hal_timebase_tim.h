/*
 * stm32f4xx_hal_timebase_tim.h
 *
 *  Created on: 2023. jul. 15.
 *      Author: Balint
 */

#ifndef _STM32F4XX_HAL_TIMEBASE_TIM_H_
#define _STM32F4XX_HAL_TIMEBASE_TIM_H_

#include <stdint.h>

enum _HAL_TIMEBASE_ERROR
{
    HAL_TIMEBASE_ERROR_TIM_BASE_INIT                = 0x00000001UL,
    HAL_TIMEBASE_ERROR_REGISTER_PERIOD_ELAPSED_CB   = 0x00000002UL,
    HAL_TIMEBASE_ERROR_TIM_BASE_START_IT            = 0x00000004UL,
    HAL_TIMEBASE_ERROR_TIM_BASE_STOP_IT             = 0x00000008UL,
    HAL_TIMEBASE_ERROR_UNREGISTER_PERIOD_ELAPSED_CB = 0x00000010UL,
    HAL_TIMEBASE_ERROR_TIM_BASE_DEINIT              = 0x00000020UL,

    HAL_TIMEBASE_ERROR_UINT_MAX                     = 0xFFFFFFFFUL,
};

uint32_t hal_timebase_get_error(void);

#endif /* _STM32F4XX_HAL_TIMEBASE_TIM_H_ */
