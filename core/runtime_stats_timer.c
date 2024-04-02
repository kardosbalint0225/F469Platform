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
 * @file        runtime_stats_timer.c
 * @brief       Timer for Runtime Statistics
 */

#include "runtime_stats_timer.h"
#include "FreeRTOS.h"
#include "stm32f4xx_hal.h"
#include "rcc.h"

static TIM_HandleTypeDef h_runtime_stats_timer;
static volatile uint32_t _count;
static void period_elapsed_cb(TIM_HandleTypeDef *htim);

void runtime_stats_timer_init(void)
{
    RCC_ClkInitTypeDef clock_config;
    uint32_t tim_clock = 0ul;
    uint32_t apb1_prescaler = 0ul;

    uint32_t tim_prescaler = 0ul;
    uint32_t flash_latency;

    _count = 0ul;

    rcc_periph_clk_enable((const void *)RUNTIME_STATS_TIMER_TIMx);
    rcc_periph_reset((const void *)RUNTIME_STATS_TIMER_TIMx);

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

    /* Compute the prescaler value to have TIMx counter clock equal to 1MHz */
    tim_prescaler = (uint32_t)((tim_clock / 1000000ul) - 1ul);

    /* Initialize TIMx peripheral as follow:
     *
     + Period = [(TIMxCLK/10000) - 1]. to have a (1/10000) s time base.

     + Prescaler = (tim_clock/1000000 - 1) to have a 1MHz counter clock.
     + ClockDivision = 0
     + Counter direction = Up
     */
    h_runtime_stats_timer.Instance = RUNTIME_STATS_TIMER_TIMx;
    h_runtime_stats_timer.Init.Period = (1000000ul / 10000ul) - 1ul;
    h_runtime_stats_timer.Init.Prescaler = tim_prescaler;
    h_runtime_stats_timer.Init.ClockDivision = 0ul;
    h_runtime_stats_timer.Init.CounterMode = TIM_COUNTERMODE_UP;

    HAL_StatusTypeDef ret;
    ret = HAL_TIM_Base_Init(&h_runtime_stats_timer);
    assert(HAL_OK == ret);

    ret = HAL_TIM_RegisterCallback(&h_runtime_stats_timer, HAL_TIM_PERIOD_ELAPSED_CB_ID, period_elapsed_cb);
    assert(HAL_OK == ret);

    ret = HAL_TIM_Base_Start_IT(&h_runtime_stats_timer);
    assert(HAL_OK == ret);

    HAL_NVIC_SetPriority(RUNTIME_STATS_TIMER_IRQn, RUNTIME_STATS_TIMER_IRQ_PRIORITY, 0ul);
    HAL_NVIC_EnableIRQ(RUNTIME_STATS_TIMER_IRQn);
}

void runtime_stats_timer_deinit(void)
{
    HAL_StatusTypeDef ret;

    ret = HAL_TIM_Base_Stop_IT(&h_runtime_stats_timer);
    assert(HAL_OK == ret);

    HAL_NVIC_DisableIRQ(RUNTIME_STATS_TIMER_IRQn);

    ret = HAL_TIM_UnRegisterCallback(&h_runtime_stats_timer, HAL_TIM_PERIOD_ELAPSED_CB_ID);
    assert(HAL_OK == ret);

    ret = HAL_TIM_Base_DeInit(&h_runtime_stats_timer);
    assert(HAL_OK == ret);

    rcc_periph_clk_disable((const void *)RUNTIME_STATS_TIMER_TIMx);
}

uint32_t runtime_stats_timer_get_count(void)
{
    return _count;
}

/**
 * @brief  Runtime Statistics Timer Period elapsed Callback
 * @note   This function is called by the HAL library when the timer
 *         reaches its period (10 kHz). By generating
 *         interrupt in every 0.1 ms the run time statistics can be more
 *         accurately calculated.
 */
static void period_elapsed_cb(TIM_HandleTypeDef *htim)
{
    (void)htim;
    _count++;
}

/**
 * @brief Runtime Statistics Timer Interrupt Handler
 */
void RUNTIME_STATS_TIMER_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&h_runtime_stats_timer);
}
/** @} */
