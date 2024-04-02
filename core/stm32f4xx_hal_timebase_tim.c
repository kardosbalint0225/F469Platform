/*
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
 * @{
 * @file        stm32f4xx_hal_timebase_tim.c
 * @brief       HAL Timebase Configuration
 */
#include "stm32f4xx_hal.h"
#include "rcc.h"

static TIM_HandleTypeDef h_hal_timebase_tim;
static void period_elapsed_cb(TIM_HandleTypeDef *htim);

/**
 * @brief     This function configures the HAL_TIMEBASE_TIM defined in stm32f4xx_hal_conf.h
 *            as the HAL time base source.
 *            The time source is configured to have 1 ms time base with a dedicated
 *            Tick interrupt priority.
 * @note      This function is called  automatically at the beginning of program after
 *            reset by HAL_Init() or at any time when clock is configured, by HAL_RCC_ClockConfig().
 * @param[in] TickPriority: Tick interrupt priority.
 * @return    HAL status
 */
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
    RCC_ClkInitTypeDef clock_config;
    uint32_t tim_clock;
    uint32_t apb1_prescaler = 0ul;

    uint32_t tim_prescaler = 0ul;
    uint32_t flash_latency;
    HAL_StatusTypeDef ret;

    uwTickPrio = TickPriority;

    rcc_periph_clk_enable((const void *)HAL_TIMEBASE_TIMx);
    rcc_periph_reset((const void *)HAL_TIMEBASE_TIMx);

    HAL_RCC_GetClockConfig(&clock_config, &flash_latency);

    apb1_prescaler = clock_config.APB1CLKDivider;

    if (RCC_HCLK_DIV1 == apb1_prescaler)
    {
        tim_clock = HAL_RCC_GetPCLK1Freq();
    }
    else
    {
        tim_clock = 2ul * HAL_RCC_GetPCLK1Freq();
    }

    /* Compute the prescaler value to have TIM counter clock equal to 1MHz */
    tim_prescaler = (uint32_t)((tim_clock / 1000000ul) - 1ul);


    /* Initialize TIMx peripheral as follow:
     + Period = [(TIMxCLK/1000) - 1]. to have a (1/1000) s time base.
     + Prescaler = (tim_clock/1000000 - 1) to have a 1MHz counter clock.
     + ClockDivision = 0
     + Counter direction = Up
     */
    h_hal_timebase_tim.Instance = HAL_TIMEBASE_TIMx;
    h_hal_timebase_tim.Init.Period = (1000000ul / 1000ul) - 1ul;
    h_hal_timebase_tim.Init.Prescaler = tim_prescaler;
    h_hal_timebase_tim.Init.ClockDivision = 0ul;
    h_hal_timebase_tim.Init.CounterMode = TIM_COUNTERMODE_UP;
    h_hal_timebase_tim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    ret = HAL_TIM_Base_Init(&h_hal_timebase_tim);
    if (HAL_OK != ret)
    {
        return ret;
    }

    ret = HAL_TIM_RegisterCallback(&h_hal_timebase_tim, HAL_TIM_PERIOD_ELAPSED_CB_ID, period_elapsed_cb);
    if (HAL_OK != ret)
    {
        return ret;
    }

    ret = HAL_TIM_Base_Start_IT(&h_hal_timebase_tim);
    if (HAL_OK != ret)
    {
        return ret;
    }

    assert(TickPriority < (1ul << __NVIC_PRIO_BITS));

    HAL_NVIC_SetPriority(HAL_TIMEBASE_TIM_IRQn, TickPriority, 0ul);
    HAL_NVIC_EnableIRQ(HAL_TIMEBASE_TIM_IRQn);

    return ret;
}

/**
 * @brief  Suspend Tick increment.
 *
 * Disable the tick increment by disabling the timer update interrupt.
 */
void HAL_SuspendTick(void)
{
    __HAL_TIM_DISABLE_IT(&h_hal_timebase_tim, TIM_IT_UPDATE);
}

/**
 * @brief  Resume Tick increment.
 *
 * Enable the tick increment by enabling the timer update interrupt.
 */
void HAL_ResumeTick(void)
{
    __HAL_TIM_ENABLE_IT(&h_hal_timebase_tim, TIM_IT_UPDATE);
}

/**
 * @brief  Period elapsed callback in non blocking mode
 *
 * This function is called when HAL_TIMEBASE_TIM interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 *
 * @param  htim : TIM handle
 *
 */
static void period_elapsed_cb(TIM_HandleTypeDef *htim)
{
    (void)htim;
    HAL_IncTick();
}

/**
 * @brief HAL Timebase Timer Interrupt Handler
 */
void HAL_TIMEBASE_TIM_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&h_hal_timebase_tim);
}
/** @} */
