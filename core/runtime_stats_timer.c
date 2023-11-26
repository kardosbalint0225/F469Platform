/*
 * run_time_stats.c
 *
 *  Created on: 2022. apr. 20.
 *      Author: Balint
 */
#include "runtime_stats_timer.h"
#include "stm32f4xx_hal.h"
#include "hal_errno.h"
#include <errno.h>

TIM_HandleTypeDef h_tim2;
static volatile uint32_t _count;

static int tim2_init(void);
static int tim2_deinit(void);
static void tim2_period_elapsed_callback(TIM_HandleTypeDef *htim);

/**
 * @brief  Configures a dedicated Timer peripheral for
 *         generating the Run-Time Stats
 * @param  None
 * @retval None
 * @note   This function is called by the FreeRTOS kernel
 */
void runtime_stats_timer_init(void)
{
    tim2_init();
}

/**
 * @brief  Deinitializes the Runtime Stats Timer
 * @param  None
 * @retval None
 * @note   -
 */
void runtime_stats_timer_deinit(void)
{
    tim2_deinit();
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
 * @brief  TIM2 peripheral initialization
 *
 * @param  None
 *
 * @return  0 for success
 * @return < 0 an error occurred
 *
 * @note   The TIM2 interrupt priority level is set to 11
 */
static int tim2_init(void)
{
    RCC_ClkInitTypeDef clkconfig;
    uint32_t uwTimclock = 0ul;
    uint32_t uwPrescalerValue = 0ul;
    uint32_t pFLatency;

    _count = 0ul;

    HAL_NVIC_SetPriority(TIM2_IRQn, 11ul, 0ul);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);

    __HAL_RCC_TIM2_CLK_ENABLE();

    HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);

    if (RCC_HCLK_DIV1 == clkconfig.APB1CLKDivider)
    {
        uwTimclock = HAL_RCC_GetPCLK1Freq();
    }
    else
    {
        uwTimclock = 2ul * HAL_RCC_GetPCLK1Freq();
    }

    /* Compute the prescaler value to have TIM2 counter clock equal to 1MHz */
    uwPrescalerValue = (uint32_t)((uwTimclock / 1000000ul) - 1ul);

    /* Initialize TIMx peripheral as follow:
     *
     + Period = [(TIM2CLK/10000) - 1]. to have a (1/10000) s time base.

     + Prescaler = (uwTimclock/1000000 - 1) to have a 1MHz counter clock.
     + ClockDivision = 0
     + Counter direction = Up
     */
    h_tim2.Instance = TIM2;
    h_tim2.Init.Period = (1000000ul / 10000ul) - 1ul;
    h_tim2.Init.Prescaler = uwPrescalerValue;
    h_tim2.Init.ClockDivision = 0ul;
    h_tim2.Init.CounterMode = TIM_COUNTERMODE_UP;

    HAL_StatusTypeDef ret;
    ret = HAL_TIM_Base_Init(&h_tim2);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_TIM_RegisterCallback(&h_tim2, HAL_TIM_PERIOD_ELAPSED_CB_ID, tim2_period_elapsed_callback);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_TIM_Base_Start_IT(&h_tim2);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    return 0;
}

/**
 * @brief  TIM2 peripheral de-initialization
 *
 * @param  None
 *
 * @return  0 for success
 * @return < 0 an error occurred
 *
 */
static int tim2_deinit(void)
{
    HAL_StatusTypeDef ret;

    /* Stop the TIM time Base generation in interrupt mode */
    ret = HAL_TIM_Base_Stop_IT(&h_tim2);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    /* Disable the TIM2 global Interrupt */
    HAL_NVIC_DisableIRQ(TIM2_IRQn);

    ret = HAL_TIM_UnRegisterCallback(&h_tim2, HAL_TIM_PERIOD_ELAPSED_CB_ID);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_TIM_Base_DeInit(&h_tim2);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    /* Disable TIM2 clock */
    __HAL_RCC_TIM2_CLK_DISABLE();

    __HAL_RCC_TIM2_FORCE_RESET();
    __HAL_RCC_TIM2_RELEASE_RESET();

    return 0;
}

/**
 * @brief  TIM2 Period elapsed callback
 * @param  None
 * @retval None
 * @note   This function is called from the HAL library
 * @note   This function is called when the timer
 *         reaches its predefined period (10 kHz). By generating
 *         interrupt in every 0.1 ms the run time stats can be more
 *         accurately calculated.
 */
static void tim2_period_elapsed_callback(TIM_HandleTypeDef *htim)
{
    _count++;
}

