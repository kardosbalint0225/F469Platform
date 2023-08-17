/*
 * sysclk.c
 *
 *  Created on: 2023. jul. 6.
 *      Author: Balint
 */
#include "stm32f4xx_hal.h"
#include "sysclk.h"

static uint32_t sysclk_error;

/**
 * @brief  System Clock Configuration
 * @retval None
 */
void sysclk_init(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    sysclk_error = 0UL;

    /** Configure the main internal regulator output voltage
     */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 4;
    RCC_OscInitStruct.PLL.PLLN = 180;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 6;
    RCC_OscInitStruct.PLL.PLLR = 6;

    HAL_StatusTypeDef ret;
    ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
    sysclk_error |= (HAL_OK != ret) ? SYSCLK_ERROR_RCC_OSC_CONFIG : 0UL;
    assert_param(0UL == sysclk_error);

    /** Activate the Over-Drive mode
     */
    ret = HAL_PWREx_EnableOverDrive();
    sysclk_error |= (HAL_OK != ret) ? SYSCLK_ERROR_PWR_ENABLE_OVERDRIVE : 0UL;
    assert_param(0UL == sysclk_error);

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
    sysclk_error |= (HAL_OK != ret) ? SYSCLK_ERROR_RCC_CLOCK_CONFIG : 0UL;
    assert_param(0UL == sysclk_error);
}

/**
 * @brief  Gets the current error state of the Sysclk
 * @param  None
 * @retval 0 if no error occured
 *         positive value indicates error where each bit
 *         corresponds to a specific error defined in _SYSCLK_ERROR
 * @note   -
 */
uint32_t sysclk_get_error(void)
{
    return sysclk_error;
}
