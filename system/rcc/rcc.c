/*
 * rcc.c
 *
 *  Created on: Feb 25, 2024
 *      Author: Balint
 */
#include "rcc.h"

HAL_StatusTypeDef rtc_clock_source_init(void)
{
    RCC_PeriphCLKInitTypeDef rtc_clock = {
        .PeriphClockSelection = RCC_PERIPHCLK_RTC,
        .RTCClockSelection = RCC_RTCCLKSOURCE_LSE,
    };
    return HAL_RCCEx_PeriphCLKConfig(&rtc_clock);
}

HAL_StatusTypeDef rtc_clock_source_deinit(void)
{
    RCC_PeriphCLKInitTypeDef rtc_clock = {
        .PeriphClockSelection = RCC_PERIPHCLK_RTC,
        .RTCClockSelection = RCC_RTCCLKSOURCE_NO_CLK,
    };
    return HAL_RCCEx_PeriphCLKConfig(&rtc_clock);
}

HAL_StatusTypeDef clk48_clock_init(void)
{
    RCC_PeriphCLKInitTypeDef clk48_clock = {
        .PeriphClockSelection = RCC_PERIPHCLK_CLK48,
        .Clk48ClockSelection = RCC_CLK48CLKSOURCE_PLLSAIP,
        .PLLSAI.PLLSAIN = 384,
        .PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV8,
    };
    return HAL_RCCEx_PeriphCLKConfig(&clk48_clock);
}

HAL_StatusTypeDef clk48_clock_deinit(void)
{
    return HAL_OK;
}

HAL_StatusTypeDef sdio_clock_source_init(void)
{
    RCC_PeriphCLKInitTypeDef sdio_clock = {
        .PeriphClockSelection = RCC_PERIPHCLK_SDIO,
        .SdioClockSelection = RCC_SDIOCLKSOURCE_CLK48,
    };

    return HAL_RCCEx_PeriphCLKConfig(&sdio_clock);
}

HAL_StatusTypeDef sdio_clock_source_deinit(void)
{
    return HAL_OK;
}

void gpio_a_clk_enable_disable(const clk_state_t state)
{
    if (GPIO_CLK_ENABLE == state)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_GPIOA_CLK_DISABLE();
    }
}

void gpio_b_clk_enable_disable(const clk_state_t state)
{
    if (GPIO_CLK_ENABLE == state)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_GPIOB_CLK_DISABLE();
    }
}

void gpio_c_clk_enable_disable(const clk_state_t state)
{
    if (GPIO_CLK_ENABLE == state)
    {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_GPIOC_CLK_DISABLE();
    }
}

void gpio_d_clk_enable_disable(const clk_state_t state)
{
    if (GPIO_CLK_ENABLE == state)
    {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_GPIOD_CLK_DISABLE();
    }
}

void gpio_e_clk_enable_disable(const clk_state_t state)
{
    if (GPIO_CLK_ENABLE == state)
    {
        __HAL_RCC_GPIOE_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_GPIOE_CLK_DISABLE();
    }
}

void gpio_f_clk_enable_disable(const clk_state_t state)
{
    if (GPIO_CLK_ENABLE == state)
    {
        __HAL_RCC_GPIOF_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_GPIOF_CLK_DISABLE();
    }
}

void gpio_g_clk_enable_disable(const clk_state_t state)
{
    if (GPIO_CLK_ENABLE == state)
    {
        __HAL_RCC_GPIOG_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_GPIOG_CLK_DISABLE();
    }
}

void gpio_h_clk_enable_disable(const clk_state_t state)
{
    if (GPIO_CLK_ENABLE == state)
    {
        __HAL_RCC_GPIOH_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_GPIOH_CLK_DISABLE();
    }
}

void gpio_i_clk_enable_disable(const clk_state_t state)
{
    if (GPIO_CLK_ENABLE == state)
    {
        __HAL_RCC_GPIOI_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_GPIOI_CLK_DISABLE();
    }
}

void gpio_j_clk_enable_disable(const clk_state_t state)
{
    if (GPIO_CLK_ENABLE == state)
    {
        __HAL_RCC_GPIOJ_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_GPIOJ_CLK_DISABLE();
    }
}

void gpio_k_clk_enable_disable(const clk_state_t state)
{
    if (GPIO_CLK_ENABLE == state)
    {
        __HAL_RCC_GPIOK_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_GPIOK_CLK_DISABLE();
    }
}
