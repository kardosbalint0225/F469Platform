/*
 * run_time_stats.c
 *
 *  Created on: 2022. apr. 20.
 *      Author: Balint
 */
#include "stm32f4xx_hal.h"

TIM_HandleTypeDef htim2;
volatile uint32_t runtime_stats_timer;

static void TIM2_Init(void);
static void TIM2_Deinit(void);
static void TIM2_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

/**
  * @brief  Configures a dedicated Timer peripheral for
  *  		generating the Run-Time Stats
  * @param  None
  * @retval None
  * @note	This function is called by the FreeRTOS kernel
  */
void run_time_stats_timer_init( void )
{
	TIM2_Init();
}

void run_time_stats_timer_deinit(void)
{
	TIM2_Deinit();
}

/**
  * @brief	TIM2 peripheral initialization
  * @param  None
  * @retval None
  * @note   The TIM2 interrupt priority level is set to 11
  */
static void TIM2_Init(void)
{
	RCC_ClkInitTypeDef clkconfig;
	uint32_t           uwTimclock = 0;
	uint32_t           uwPrescalerValue = 0;
	uint32_t           pFLatency;

	runtime_stats_timer = 0;

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
	uwPrescalerValue = (uint32_t) ((uwTimclock / 1000000U) - 1U);

	/* Initialize TIM2 */
	htim2.Instance = TIM2;

	/* Initialize TIMx peripheral as follow:
	 *
	+ Period = [(TIM2CLK/10000) - 1]. to have a (1/10000) s time base.

	+ Prescaler = (uwTimclock/1000000 - 1) to have a 1MHz counter clock.
	+ ClockDivision = 0
	+ Counter direction = Up
	*/
	htim2.Init.Period        = (1000000U / 10000U) - 1U;
	htim2.Init.Prescaler     = uwPrescalerValue;
	htim2.Init.ClockDivision = 0;
	htim2.Init.CounterMode   = TIM_COUNTERMODE_UP;

	HAL_StatusTypeDef ret;
	ret = HAL_TIM_Base_Init(&htim2);
	assert_param(HAL_OK == ret);

	ret = HAL_TIM_RegisterCallback(&htim2, HAL_TIM_PERIOD_ELAPSED_CB_ID, TIM2_PeriodElapsedCallback);
	assert_param(HAL_OK == ret);

	/* Start the TIM time Base generation in interrupt mode */
	ret = HAL_TIM_Base_Start_IT(&htim2);
	assert_param(HAL_OK == ret);
}

/**
  * @brief 	TIM2 peripheral de-initialization
  * @param  None
  * @retval None
  * @note
  */
static void TIM2_Deinit(void)
{
	HAL_StatusTypeDef ret;

	/* Stop the TIM time Base generation in interrupt mode */
	ret = HAL_TIM_Base_Stop_IT(&htim2);
	assert_param(HAL_OK == ret);

	/* Disable the TIM2 global Interrupt */
	HAL_NVIC_DisableIRQ(TIM2_IRQn);

	ret = HAL_TIM_UnRegisterCallback(&htim2, HAL_TIM_PERIOD_ELAPSED_CB_ID);
	assert_param(HAL_OK == ret);

	ret = HAL_TIM_Base_DeInit(&htim2);
	assert_param(HAL_OK == ret);

	/* Disable TIM2 clock */
	__HAL_RCC_TIM2_CLK_DISABLE();

	__HAL_RCC_TIM2_FORCE_RESET();
	__HAL_RCC_TIM2_RELEASE_RESET();
}

/**
  * @brief	TIM2 Period elapsed callback
  * @param  None
  * @retval None
  * @note	This function is called from the HAL library
  * @note 	This function is called when the timer
  * 		reaches its predefined period (10 kHz). By generating
  * 		interrupt in every 0.1 ms the run time stats can be more
  * 		accurately calculated.
  */
static void TIM2_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	runtime_stats_timer++;
}




