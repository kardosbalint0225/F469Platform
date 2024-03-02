#include "system_clock.h"
#include "stm32f4xx_hal.h"
#include "hal_errno.h"

/**
 * @brief  Initializes the HSE and LSE oscillators
 *
 * @return 0 on success
 * @return < 0 on error
 */
int system_clock_oscillator_init(void)
{
    RCC_OscInitTypeDef osc_config = { 0 };

    osc_config.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
    osc_config.HSEState = RCC_HSE_ON;
    osc_config.LSEState = RCC_LSE_ON;
    osc_config.PLL.PLLState = RCC_PLL_ON;
    osc_config.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    osc_config.PLL.PLLM = 8ul;
    osc_config.PLL.PLLN = 360ul;
    osc_config.PLL.PLLP = RCC_PLLP_DIV2;
    osc_config.PLL.PLLQ = 6ul;
    osc_config.PLL.PLLR = 6ul;

    HAL_StatusTypeDef ret;
    ret = HAL_RCC_OscConfig(&osc_config);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_PWREx_EnableOverDrive();
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    return 0;
}

/**
 * @brief  Initializes the CPU, AHB and APB buses clocks
 *
 * @return 0 on success
 * @return < 0 on error
 */
int system_clock_bus_clock_init(void)
{
    RCC_ClkInitTypeDef clk_config = { 0 };

    clk_config.ClockType = RCC_CLOCKTYPE_HCLK |
                           RCC_CLOCKTYPE_SYSCLK |
                           RCC_CLOCKTYPE_PCLK1 |
                           RCC_CLOCKTYPE_PCLK2;
    clk_config.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk_config.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk_config.APB1CLKDivider = RCC_HCLK_DIV4;
    clk_config.APB2CLKDivider = RCC_HCLK_DIV2;

    HAL_StatusTypeDef ret;
    ret = HAL_RCC_ClockConfig(&clk_config, FLASH_LATENCY_5);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    return 0;
}

/**
 * @brief  System Clock Configuration
 *
 * @return 0 on success
 * @return < 0 on error
 */
int system_clock_init(void)
{
    /** Configure the main internal regulator output voltage */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    int ret;
    ret = system_clock_oscillator_init();
    if (ret != 0)
    {
        return ret;
    }

    ret = system_clock_bus_clock_init();
    if (ret != 0)
    {
        return ret;
    }

    return 0;
}

