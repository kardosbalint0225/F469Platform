/*
 * rcc.c
 *
 *  Created on: Feb 25, 2024
 *      Author: Balint
 */
#include "rcc.h"

static int find_tim_index(const TIM_TypeDef *tim);
static int find_usart_index(const USART_TypeDef *usart);

typedef struct {
    const TIM_TypeDef * const base_address;
    const void (* const clk_enable_disable_fn)(const periph_clock_state_t state);
    const void (* const periph_reset_fn)(void);
} rcc_tim_t;

typedef struct {
    const USART_TypeDef * const base_address;
    const void (* const clk_enable_disable_fn)(const periph_clock_state_t state);
    const void (* const periph_reset_fn)(void);
} rcc_usart_t;

static const rcc_usart_t _usart[] = {
    { USART1, rcc_usart1_clk_enable_disable, rcc_usart1_periph_reset },
    { USART2, rcc_usart2_clk_enable_disable, rcc_usart2_periph_reset },
    { USART3, rcc_usart3_clk_enable_disable, rcc_usart3_periph_reset },
    { UART4, rcc_uart4_clk_enable_disable, rcc_uart4_periph_reset },
    { UART5, rcc_uart5_clk_enable_disable, rcc_uart5_periph_reset},
    { USART6, rcc_usart6_clk_enable_disable, rcc_usart6_periph_reset },
    { UART7, rcc_uart7_clk_enable_disable, rcc_uart7_periph_reset },
    { UART8, rcc_uart8_clk_enable_disable, rcc_uart8_periph_reset },
};

static const rcc_tim_t _tim[] = {
    { TIM1, rcc_tim1_clk_enable_disable, rcc_tim1_periph_reset },
    { TIM2, rcc_tim2_clk_enable_disable, rcc_tim2_periph_reset },
    { TIM3, rcc_tim3_clk_enable_disable, rcc_tim3_periph_reset },
    { TIM4, rcc_tim4_clk_enable_disable, rcc_tim4_periph_reset },
    { TIM5, rcc_tim5_clk_enable_disable, rcc_tim5_periph_reset },
    { TIM6, rcc_tim6_clk_enable_disable, rcc_tim6_periph_reset },
    { TIM7, rcc_tim7_clk_enable_disable, rcc_tim7_periph_reset },
    { TIM8, rcc_tim8_clk_enable_disable, rcc_tim8_periph_reset },
    { TIM9, rcc_tim9_clk_enable_disable, rcc_tim9_periph_reset },
    { TIM10, rcc_tim10_clk_enable_disable, rcc_tim10_periph_reset },
    { TIM11, rcc_tim11_clk_enable_disable, rcc_tim11_periph_reset },
    { TIM12, rcc_tim12_clk_enable_disable, rcc_tim12_periph_reset },
    { TIM13, rcc_tim13_clk_enable_disable, rcc_tim13_periph_reset },
    { TIM14, rcc_tim14_clk_enable_disable, rcc_tim14_periph_reset },
};

void rcc_timx_clk_enable(const TIM_TypeDef *tim)
{
    assert(tim);

    const int i = find_tim_index(tim);
    assert(-1 != i);

    _tim[i].clk_enable_disable_fn(PERIPH_CLOCK_ENABLE);
}

void rcc_timx_clk_disable(const TIM_TypeDef *tim)
{
    assert(tim);

    const int i = find_tim_index(tim);
    assert(-1 != i);

    _tim[i].clk_enable_disable_fn(PERIPH_CLOCK_DISABLE);
}

void rcc_timx_periph_reset(const TIM_TypeDef *tim)
{
    assert(tim);

    const int i = find_tim_index(tim);
    assert(-1 != i);

    _tim[i].periph_reset_fn();
}

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

void rcc_gpioa_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_GPIOA_CLK_DISABLE();
    }
}

void rcc_gpiob_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_GPIOB_CLK_DISABLE();
    }
}

void rcc_gpioc_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_GPIOC_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_GPIOC_CLK_DISABLE();
    }
}

void rcc_gpiod_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_GPIOD_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_GPIOD_CLK_DISABLE();
    }
}

void rcc_gpioe_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_GPIOE_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_GPIOE_CLK_DISABLE();
    }
}

void rcc_gpiof_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_GPIOF_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_GPIOF_CLK_DISABLE();
    }
}

void rcc_gpiog_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_GPIOG_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_GPIOG_CLK_DISABLE();
    }
}

void rcc_gpioh_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_GPIOH_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_GPIOH_CLK_DISABLE();
    }
}

void rcc_gpioi_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_GPIOI_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_GPIOI_CLK_DISABLE();
    }
}

void rcc_gpioj_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_GPIOJ_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_GPIOJ_CLK_DISABLE();
    }
}

void rcc_gpiok_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_GPIOK_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_GPIOK_CLK_DISABLE();
    }
}

void rcc_tim1_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_TIM1_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_TIM1_CLK_DISABLE();
    }
}

void rcc_tim2_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_TIM2_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_TIM2_CLK_DISABLE();
    }
}

void rcc_tim3_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_TIM3_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_TIM3_CLK_DISABLE();
    }
}

void rcc_tim4_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_TIM4_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_TIM4_CLK_DISABLE();
    }
}

void rcc_tim5_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_TIM5_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_TIM5_CLK_DISABLE();
    }
}

void rcc_tim6_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_TIM6_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_TIM6_CLK_DISABLE();
    }
}

void rcc_tim7_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_TIM7_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_TIM7_CLK_DISABLE();
    }
}

void rcc_tim8_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_TIM8_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_TIM8_CLK_DISABLE();
    }
}

void rcc_tim9_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_TIM9_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_TIM9_CLK_DISABLE();
    }
}

void rcc_tim10_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_TIM10_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_TIM10_CLK_DISABLE();
    }
}

void rcc_tim11_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_TIM11_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_TIM11_CLK_DISABLE();
    }
}

void rcc_tim12_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_TIM12_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_TIM12_CLK_DISABLE();
    }
}

void rcc_tim13_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_TIM13_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_TIM13_CLK_DISABLE();
    }
}

void rcc_tim14_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_TIM14_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_TIM14_CLK_DISABLE();
    }
}

void rcc_tim1_periph_reset(void)
{
    __HAL_RCC_TIM1_FORCE_RESET();
    __HAL_RCC_TIM1_RELEASE_RESET();
}

void rcc_tim2_periph_reset(void)
{
    __HAL_RCC_TIM2_FORCE_RESET();
    __HAL_RCC_TIM2_RELEASE_RESET();
}

void rcc_tim3_periph_reset(void)
{
    __HAL_RCC_TIM3_FORCE_RESET();
    __HAL_RCC_TIM3_RELEASE_RESET();
}

void rcc_tim4_periph_reset(void)
{
    __HAL_RCC_TIM4_FORCE_RESET();
    __HAL_RCC_TIM4_RELEASE_RESET();
}

void rcc_tim5_periph_reset(void)
{
    __HAL_RCC_TIM5_FORCE_RESET();
    __HAL_RCC_TIM5_RELEASE_RESET();
}

void rcc_tim6_periph_reset(void)
{
    __HAL_RCC_TIM6_FORCE_RESET();
    __HAL_RCC_TIM6_RELEASE_RESET();
}

void rcc_tim7_periph_reset(void)
{
    __HAL_RCC_TIM7_FORCE_RESET();
    __HAL_RCC_TIM7_RELEASE_RESET();
}

void rcc_tim8_periph_reset(void)
{
    __HAL_RCC_TIM8_FORCE_RESET();
    __HAL_RCC_TIM8_RELEASE_RESET();
}

void rcc_tim9_periph_reset(void)
{
    __HAL_RCC_TIM9_FORCE_RESET();
    __HAL_RCC_TIM9_RELEASE_RESET();
}

void rcc_tim10_periph_reset(void)
{
    __HAL_RCC_TIM10_FORCE_RESET();
    __HAL_RCC_TIM10_RELEASE_RESET();
}

void rcc_tim11_periph_reset(void)
{
    __HAL_RCC_TIM11_FORCE_RESET();
    __HAL_RCC_TIM11_RELEASE_RESET();
}

void rcc_tim12_periph_reset(void)
{
    __HAL_RCC_TIM12_FORCE_RESET();
    __HAL_RCC_TIM12_RELEASE_RESET();
}

void rcc_tim13_periph_reset(void)
{
    __HAL_RCC_TIM13_FORCE_RESET();
    __HAL_RCC_TIM13_RELEASE_RESET();
}

void rcc_tim14_periph_reset(void)
{
    __HAL_RCC_TIM14_FORCE_RESET();
    __HAL_RCC_TIM14_RELEASE_RESET();
}

void rcc_usartx_clk_enable(const USART_TypeDef *usart)
{
    assert(usart);

    const int i = find_usart_index(usart);
    assert(-1 != i);

    _usart[i].clk_enable_disable_fn(PERIPH_CLOCK_ENABLE);
}

void rcc_usartx_clk_disable(const USART_TypeDef *usart)
{
    assert(usart);

    const int i = find_usart_index(usart);
    assert(-1 != i);

    _usart[i].clk_enable_disable_fn(PERIPH_CLOCK_DISABLE);
}

void rcc_usartx_periph_reset(const USART_TypeDef *usart)
{
    assert(usart);

    const int i = find_usart_index(usart);
    assert(-1 != i);

    _usart[i].periph_reset_fn();
}

void rcc_usart1_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_USART1_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_USART1_CLK_DISABLE();
    }
}

void rcc_usart2_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_USART2_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_USART2_CLK_DISABLE();
    }
}

void rcc_usart3_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_USART3_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_USART3_CLK_DISABLE();
    }
}

void rcc_uart4_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_UART4_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_UART4_CLK_DISABLE();
    }
}

void rcc_uart5_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_UART5_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_UART5_CLK_DISABLE();
    }
}

void rcc_usart6_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_USART6_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_USART6_CLK_DISABLE();
    }
}

void rcc_uart7_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_UART7_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_UART7_CLK_DISABLE();
    }
}

void rcc_uart8_clk_enable_disable(const periph_clock_state_t state)
{
    if (PERIPH_CLOCK_ENABLE == state)
    {
        __HAL_RCC_UART8_CLK_ENABLE();
    }
    else
    {
        __HAL_RCC_UART8_CLK_DISABLE();
    }
}

void rcc_usart1_periph_reset(void)
{
    __HAL_RCC_USART1_FORCE_RESET();
    __HAL_RCC_USART1_RELEASE_RESET();
}

void rcc_usart2_periph_reset(void)
{
    __HAL_RCC_USART2_FORCE_RESET();
    __HAL_RCC_USART2_RELEASE_RESET();
}

void rcc_usart3_periph_reset(void)
{
    __HAL_RCC_USART3_FORCE_RESET();
    __HAL_RCC_USART3_RELEASE_RESET();
}

void rcc_uart4_periph_reset(void)
{
    __HAL_RCC_UART4_FORCE_RESET();
    __HAL_RCC_UART4_RELEASE_RESET();
}

void rcc_uart5_periph_reset(void)
{
    __HAL_RCC_UART5_FORCE_RESET();
    __HAL_RCC_UART5_RELEASE_RESET();
}

void rcc_usart6_periph_reset(void)
{
    __HAL_RCC_USART6_FORCE_RESET();
    __HAL_RCC_USART6_RELEASE_RESET();
}

void rcc_uart7_periph_reset(void)
{
    __HAL_RCC_UART7_FORCE_RESET();
    __HAL_RCC_UART7_RELEASE_RESET();
}

void rcc_uart8_periph_reset(void)
{
    __HAL_RCC_UART8_FORCE_RESET();
    __HAL_RCC_UART8_RELEASE_RESET();
}

static int find_tim_index(const TIM_TypeDef *tim)
{
    int index = -1;
    const int size = sizeof(_tim) / sizeof(rcc_tim_t);

    for (int i = 0; i < size; i++)
    {
        if (tim == _tim[i].base_address)
        {
            index = i;
            break;
        }
    }

    return index;
}

static int find_usart_index(const USART_TypeDef *usart)
{
    int index = -1;
    const int size = sizeof(_usart) / sizeof(rcc_usart_t);

    for (int i = 0; i < size; i++)
    {
        if (usart == _usart[i].base_address)
        {
            index = i;
            break;
        }
    }

    return index;
}
