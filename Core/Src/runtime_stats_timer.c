/*
 * run_time_stats.c
 *
 *  Created on: 2022. apr. 20.
 *      Author: Balint
 */
#include "runtime_stats_timer.h"
#include "stm32f4xx_hal.h"

TIM_HandleTypeDef htim2;
static uint32_t runtime_stats_timer_error;
static volatile uint32_t runtime_stats_timer;

static void tim2_init(void);
static void tim2_deinit(void);
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
    return runtime_stats_timer;
}

/**
 * @brief  Gets the current error state of the Runtime stats Timer
 * @param  None
 * @retval 0 if no error occured
 *         positive value indicates error where each bit
 *         corresponds to a specific error defined in _RUNTIME_STATS_TIMER_ERROR
 * @note   -
 */
uint32_t runtime_stats_timer_get_error(void)
{
    return runtime_stats_timer_error;
}

/**
 * @brief  TIM2 peripheral initialization
 * @param  None
 * @retval None
 * @note   The TIM2 interrupt priority level is set to 11
 */
static void tim2_init(void)
{
    RCC_ClkInitTypeDef clkconfig;
    uint32_t uwTimclock = 0UL;
    uint32_t uwPrescalerValue = 0UL;
    uint32_t pFLatency;

    runtime_stats_timer = 0UL;
    runtime_stats_timer_error = 0UL;

    /*Configure the TIM2 IRQ priority */
    HAL_NVIC_SetPriority(TIM2_IRQn, 11, 0);

    /* Enable the TIM2 global Interrupt */
    HAL_NVIC_EnableIRQ(TIM2_IRQn);

    /* Enable TIM2 clock */
    __HAL_RCC_TIM2_CLK_ENABLE();

    /* Get clock configuration */
    HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);

    /* Compute TIM2 clock */
    if (RCC_HCLK_DIV1 == clkconfig.APB1CLKDivider)
    {
        uwTimclock = HAL_RCC_GetPCLK1Freq();
    }
    else
    {
        uwTimclock = 2UL * HAL_RCC_GetPCLK1Freq();
    }

    /* Compute the prescaler value to have TIM2 counter clock equal to 1MHz */
    uwPrescalerValue = (uint32_t)((uwTimclock / 1000000U) - 1U);

    /* Initialize TIM2 */
    htim2.Instance = TIM2;

    /* Initialize TIMx peripheral as follow:
     *
     + Period = [(TIM2CLK/10000) - 1]. to have a (1/10000) s time base.

     + Prescaler = (uwTimclock/1000000 - 1) to have a 1MHz counter clock.
     + ClockDivision = 0
     + Counter direction = Up
     */
    htim2.Init.Period = (1000000U / 10000U) - 1U;
    htim2.Init.Prescaler = uwPrescalerValue;
    htim2.Init.ClockDivision = 0;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;

    HAL_StatusTypeDef ret;
    ret = HAL_TIM_Base_Init(&htim2);
    runtime_stats_timer_error |= (HAL_OK != ret) ? RUNTIME_STATS_TIMER_ERROR_TIM_BASE_INIT : 0UL;
    assert_param(0UL == runtime_stats_timer_error);

    ret = HAL_TIM_RegisterCallback(&htim2, HAL_TIM_PERIOD_ELAPSED_CB_ID, tim2_period_elapsed_callback);
    runtime_stats_timer_error |= (HAL_OK != ret) ? RUNTIME_STATS_TIMER_ERROR_REGISTER_PERIOD_ELAPSED_CB : 0UL;
    assert_param(0UL == runtime_stats_timer_error);

    /* Start the TIM time Base generation in interrupt mode */
    ret = HAL_TIM_Base_Start_IT(&htim2);
    runtime_stats_timer_error |= (HAL_OK != ret) ? RUNTIME_STATS_TIMER_ERROR_TIM_BASE_START_IT : 0UL;
    assert_param(0UL == runtime_stats_timer_error);
}

/**
 * @brief  TIM2 peripheral de-initialization
 * @param  None
 * @retval None
 * @note
 */
static void tim2_deinit(void)
{
    HAL_StatusTypeDef ret;

    /* Stop the TIM time Base generation in interrupt mode */
    ret = HAL_TIM_Base_Stop_IT(&htim2);
    runtime_stats_timer_error |= (HAL_OK != ret) ? RUNTIME_STATS_TIMER_ERROR_TIM_BASE_STOP_IT : 0UL;
    assert_param(0UL == runtime_stats_timer_error);

    /* Disable the TIM2 global Interrupt */
    HAL_NVIC_DisableIRQ(TIM2_IRQn);

    ret = HAL_TIM_UnRegisterCallback(&htim2, HAL_TIM_PERIOD_ELAPSED_CB_ID);
    runtime_stats_timer_error |= (HAL_OK != ret) ? RUNTIME_STATS_TIMER_ERROR_UNREGISTER_PERIOD_ELAPSED_CB : 0UL;
    assert_param(0UL == runtime_stats_timer_error);

    ret = HAL_TIM_Base_DeInit(&htim2);
    runtime_stats_timer_error |= (HAL_OK != ret) ? RUNTIME_STATS_TIMER_ERROR_TIM_BASE_DEINIT : 0UL;
    assert_param(0UL == runtime_stats_timer_error);

    /* Disable TIM2 clock */
    __HAL_RCC_TIM2_CLK_DISABLE();

    __HAL_RCC_TIM2_FORCE_RESET();
    __HAL_RCC_TIM2_RELEASE_RESET();
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
    runtime_stats_timer++;
}
