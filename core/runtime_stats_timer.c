#include "FreeRTOS.h"
#include "runtime_stats_timer.h"
#include "stm32f4xx_hal.h"
#include "hal_errno.h"
#include "rcc.h"
#include <errno.h>

TIM_HandleTypeDef h_runtime_stats_timer;
static volatile uint32_t _count;
static void period_elapsed_cb(TIM_HandleTypeDef *htim);

/**
 * @brief  Configures a dedicated Timer peripheral for
 *         generating the Run-Time Stats
 * @param  None
 * @retval None
 * @note   This function is called by the FreeRTOS kernel
 */
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

/**
 * @brief  Deinitializes the Runtime Stats Timer
 * @param  None
 * @retval None
 * @note   -
 */
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

/**
 * @brief  Returns the current timer value of the Runtime Stats Timer
 * @param  None
 * @retval 32-bit unsigned timer count ( count * 0.1 ms elapsed)
 * @note   -
 */
uint32_t runtime_stats_timer_get_count(void)
{
    return _count;
}

/**
 * @brief  TIMx Period elapsed callback
 * @param  None
 * @retval None
 * @note   This function is called from the HAL library
 * @note   This function is called when the timer
 *         reaches its predefined period (10 kHz). By generating
 *         interrupt in every 0.1 ms the run time stats can be more
 *         accurately calculated.
 */
static void period_elapsed_cb(TIM_HandleTypeDef *htim)
{
    _count++;
}

