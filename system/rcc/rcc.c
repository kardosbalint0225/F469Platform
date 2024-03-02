/*
 * rcc.c
 *
 *  Created on: Feb 25, 2024
 *      Author: Balint
 */
#include "rcc.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

static void gpioa_clk_enable(void) { __HAL_RCC_GPIOA_CLK_ENABLE(); }
static void gpiob_clk_enable(void) { __HAL_RCC_GPIOB_CLK_ENABLE(); }
static void gpioc_clk_enable(void) { __HAL_RCC_GPIOC_CLK_ENABLE(); }
static void gpiod_clk_enable(void) { __HAL_RCC_GPIOD_CLK_ENABLE(); }
static void gpioe_clk_enable(void) { __HAL_RCC_GPIOE_CLK_ENABLE(); }
static void gpiof_clk_enable(void) { __HAL_RCC_GPIOF_CLK_ENABLE(); }
static void gpiog_clk_enable(void) { __HAL_RCC_GPIOG_CLK_ENABLE(); }
static void gpioh_clk_enable(void) { __HAL_RCC_GPIOH_CLK_ENABLE(); }
static void gpioi_clk_enable(void) { __HAL_RCC_GPIOI_CLK_ENABLE(); }
static void gpioj_clk_enable(void) { __HAL_RCC_GPIOJ_CLK_ENABLE(); }
static void gpiok_clk_enable(void) { __HAL_RCC_GPIOK_CLK_ENABLE(); }
static void gpioa_clk_disable(void) { __HAL_RCC_GPIOA_CLK_DISABLE(); }
static void gpiob_clk_disable(void) { __HAL_RCC_GPIOB_CLK_DISABLE(); }
static void gpioc_clk_disable(void) { __HAL_RCC_GPIOC_CLK_DISABLE(); }
static void gpiod_clk_disable(void) { __HAL_RCC_GPIOD_CLK_DISABLE(); }
static void gpioe_clk_disable(void) { __HAL_RCC_GPIOE_CLK_DISABLE(); }
static void gpiof_clk_disable(void) { __HAL_RCC_GPIOF_CLK_DISABLE(); }
static void gpiog_clk_disable(void) { __HAL_RCC_GPIOG_CLK_DISABLE(); }
static void gpioh_clk_disable(void) { __HAL_RCC_GPIOH_CLK_DISABLE(); }
static void gpioi_clk_disable(void) { __HAL_RCC_GPIOI_CLK_DISABLE(); }
static void gpioj_clk_disable(void) { __HAL_RCC_GPIOJ_CLK_DISABLE(); }
static void gpiok_clk_disable(void) { __HAL_RCC_GPIOK_CLK_DISABLE(); }

static void tim1_clk_enable(void) { __HAL_RCC_TIM1_CLK_ENABLE(); }
static void tim2_clk_enable(void) { __HAL_RCC_TIM2_CLK_ENABLE(); }
static void tim3_clk_enable(void) { __HAL_RCC_TIM3_CLK_ENABLE(); }
static void tim4_clk_enable(void) { __HAL_RCC_TIM4_CLK_ENABLE(); }
static void tim5_clk_enable(void) { __HAL_RCC_TIM5_CLK_ENABLE(); }
static void tim6_clk_enable(void) { __HAL_RCC_TIM6_CLK_ENABLE(); }
static void tim7_clk_enable(void) { __HAL_RCC_TIM7_CLK_ENABLE(); }
static void tim8_clk_enable(void) { __HAL_RCC_TIM8_CLK_ENABLE(); }
static void tim9_clk_enable(void) { __HAL_RCC_TIM9_CLK_ENABLE(); }
static void tim10_clk_enable(void) { __HAL_RCC_TIM10_CLK_ENABLE(); }
static void tim11_clk_enable(void) { __HAL_RCC_TIM11_CLK_ENABLE(); }
static void tim12_clk_enable(void) { __HAL_RCC_TIM12_CLK_ENABLE(); }
static void tim13_clk_enable(void) { __HAL_RCC_TIM13_CLK_ENABLE(); }
static void tim14_clk_enable(void) { __HAL_RCC_TIM14_CLK_ENABLE(); }
static void tim1_clk_disable(void) { __HAL_RCC_TIM1_CLK_DISABLE(); }
static void tim2_clk_disable(void) { __HAL_RCC_TIM2_CLK_DISABLE(); }
static void tim3_clk_disable(void) { __HAL_RCC_TIM3_CLK_DISABLE(); }
static void tim4_clk_disable(void) { __HAL_RCC_TIM4_CLK_DISABLE(); }
static void tim5_clk_disable(void) { __HAL_RCC_TIM5_CLK_DISABLE(); }
static void tim6_clk_disable(void) { __HAL_RCC_TIM6_CLK_DISABLE(); }
static void tim7_clk_disable(void) { __HAL_RCC_TIM7_CLK_DISABLE(); }
static void tim8_clk_disable(void) { __HAL_RCC_TIM8_CLK_DISABLE(); }
static void tim9_clk_disable(void) { __HAL_RCC_TIM9_CLK_DISABLE(); }
static void tim10_clk_disable(void) { __HAL_RCC_TIM10_CLK_DISABLE(); }
static void tim11_clk_disable(void) { __HAL_RCC_TIM11_CLK_DISABLE(); }
static void tim12_clk_disable(void) { __HAL_RCC_TIM12_CLK_DISABLE(); }
static void tim13_clk_disable(void) { __HAL_RCC_TIM13_CLK_DISABLE(); }
static void tim14_clk_disable(void) { __HAL_RCC_TIM14_CLK_DISABLE(); }
static void tim1_periph_reset(void) { __HAL_RCC_TIM1_FORCE_RESET(); __HAL_RCC_TIM1_RELEASE_RESET(); }
static void tim2_periph_reset(void) { __HAL_RCC_TIM2_FORCE_RESET(); __HAL_RCC_TIM2_RELEASE_RESET(); }
static void tim3_periph_reset(void) { __HAL_RCC_TIM3_FORCE_RESET(); __HAL_RCC_TIM3_RELEASE_RESET(); }
static void tim4_periph_reset(void) { __HAL_RCC_TIM4_FORCE_RESET(); __HAL_RCC_TIM4_RELEASE_RESET(); }
static void tim5_periph_reset(void) { __HAL_RCC_TIM5_FORCE_RESET(); __HAL_RCC_TIM5_RELEASE_RESET(); }
static void tim6_periph_reset(void) { __HAL_RCC_TIM6_FORCE_RESET(); __HAL_RCC_TIM6_RELEASE_RESET(); }
static void tim7_periph_reset(void) { __HAL_RCC_TIM7_FORCE_RESET(); __HAL_RCC_TIM7_RELEASE_RESET(); }
static void tim8_periph_reset(void) { __HAL_RCC_TIM8_FORCE_RESET(); __HAL_RCC_TIM8_RELEASE_RESET(); }
static void tim9_periph_reset(void) { __HAL_RCC_TIM9_FORCE_RESET(); __HAL_RCC_TIM9_RELEASE_RESET(); }
static void tim10_periph_reset(void) { __HAL_RCC_TIM10_FORCE_RESET(); __HAL_RCC_TIM10_RELEASE_RESET(); }
static void tim11_periph_reset(void) { __HAL_RCC_TIM11_FORCE_RESET(); __HAL_RCC_TIM11_RELEASE_RESET(); }
static void tim12_periph_reset(void) { __HAL_RCC_TIM12_FORCE_RESET(); __HAL_RCC_TIM12_RELEASE_RESET(); }
static void tim13_periph_reset(void) { __HAL_RCC_TIM13_FORCE_RESET(); __HAL_RCC_TIM13_RELEASE_RESET(); }
static void tim14_periph_reset(void) { __HAL_RCC_TIM14_FORCE_RESET(); __HAL_RCC_TIM14_RELEASE_RESET(); }

static void usart1_clk_enable(void) { __HAL_RCC_USART1_CLK_ENABLE(); }
static void usart2_clk_enable(void) { __HAL_RCC_USART2_CLK_ENABLE(); }
static void usart3_clk_enable(void) { __HAL_RCC_USART3_CLK_ENABLE(); }
static void uart4_clk_enable(void) { __HAL_RCC_UART4_CLK_ENABLE(); }
static void uart5_clk_enable(void) { __HAL_RCC_UART5_CLK_ENABLE(); }
static void usart6_clk_enable(void) { __HAL_RCC_USART6_CLK_ENABLE(); }
static void uart7_clk_enable(void) { __HAL_RCC_UART7_CLK_ENABLE(); }
static void uart8_clk_enable(void) { __HAL_RCC_UART8_CLK_ENABLE(); }
static void usart1_clk_disable(void) { __HAL_RCC_USART1_CLK_DISABLE(); }
static void usart2_clk_disable(void) { __HAL_RCC_USART2_CLK_DISABLE(); }
static void usart3_clk_disable(void) { __HAL_RCC_USART3_CLK_DISABLE(); }
static void uart4_clk_disable(void) { __HAL_RCC_UART4_CLK_DISABLE(); }
static void uart5_clk_disable(void) { __HAL_RCC_UART5_CLK_DISABLE(); }
static void usart6_clk_disable(void) { __HAL_RCC_USART6_CLK_DISABLE(); }
static void uart7_clk_disable(void) { __HAL_RCC_UART7_CLK_DISABLE(); }
static void uart8_clk_disable(void) { __HAL_RCC_UART8_CLK_DISABLE(); }
static void usart1_periph_reset(void) { __HAL_RCC_USART1_FORCE_RESET(); __HAL_RCC_USART1_RELEASE_RESET(); }
static void usart2_periph_reset(void) { __HAL_RCC_USART2_FORCE_RESET(); __HAL_RCC_USART2_RELEASE_RESET(); }
static void usart3_periph_reset(void) { __HAL_RCC_USART3_FORCE_RESET(); __HAL_RCC_USART3_RELEASE_RESET(); }
static void uart4_periph_reset(void) { __HAL_RCC_UART4_FORCE_RESET(); __HAL_RCC_UART4_RELEASE_RESET(); }
static void uart5_periph_reset(void) { __HAL_RCC_UART5_FORCE_RESET(); __HAL_RCC_UART5_RELEASE_RESET(); }
static void usart6_periph_reset(void) { __HAL_RCC_USART6_FORCE_RESET(); __HAL_RCC_USART6_RELEASE_RESET(); }
static void uart7_periph_reset(void) { __HAL_RCC_UART7_FORCE_RESET(); __HAL_RCC_UART7_RELEASE_RESET(); }
static void uart8_periph_reset(void) { __HAL_RCC_UART8_FORCE_RESET(); __HAL_RCC_UART8_RELEASE_RESET(); }

static int find_tim_index(const TIM_TypeDef *tim);
static int find_usart_index(const USART_TypeDef *usart);
static int find_gpio_index(const GPIO_TypeDef *gpio);
static inline void gpio_lock(void);
static inline void gpio_unlock(void);

static SemaphoreHandle_t _gpio_mutex = NULL;
static StaticSemaphore_t _gpio_mutex_storage;

typedef struct {
    const TIM_TypeDef * const base_address;
    const void (* const clk_enable_fn)(void);
    const void (* const clk_disable_fn)(void);
    const void (* const periph_reset_fn)(void);
} rcc_tim_t;

typedef struct {
    const USART_TypeDef * const base_address;
    const void (* const clk_enable_fn)(void);
    const void (* const clk_disable_fn)(void);
    const void (* const periph_reset_fn)(void);
} rcc_usart_t;

typedef struct {
    const GPIO_TypeDef * const base_address;
    const void (* const clk_enable_fn)(void);
    const void (* const clk_disable_fn)(void);
    uint32_t reference_count;
} gpio_t;

static gpio_t _gpio[] = {
    { GPIOA, gpioa_clk_enable, gpioa_clk_disable },
    { GPIOB, gpiob_clk_enable, gpiob_clk_disable },
    { GPIOC, gpioc_clk_enable, gpioc_clk_disable },
    { GPIOD, gpiod_clk_enable, gpiod_clk_disable },
    { GPIOE, gpioe_clk_enable, gpioe_clk_disable },
    { GPIOF, gpiof_clk_enable, gpiof_clk_disable },
    { GPIOG, gpiog_clk_enable, gpiog_clk_disable },
    { GPIOH, gpioh_clk_enable, gpioh_clk_disable },
    { GPIOI, gpioi_clk_enable, gpioi_clk_disable },
    { GPIOJ, gpioj_clk_enable, gpioj_clk_disable },
    { GPIOK, gpiok_clk_enable, gpiok_clk_disable },
};

static const rcc_usart_t _usart[] = {
    { USART1, usart1_clk_enable, usart1_clk_disable, usart1_periph_reset },
    { USART2, usart2_clk_enable, usart2_clk_disable, usart2_periph_reset },
    { USART3, usart3_clk_enable, usart3_clk_disable, usart3_periph_reset },
    { UART4, uart4_clk_enable, uart4_clk_disable, uart4_periph_reset },
    { UART5, uart5_clk_enable, uart5_clk_disable, uart5_periph_reset},
    { USART6, usart6_clk_enable, usart6_clk_disable, usart6_periph_reset },
    { UART7, uart7_clk_enable, uart7_clk_disable, uart7_periph_reset },
    { UART8, uart8_clk_enable, uart8_clk_disable, uart8_periph_reset },
};

static const rcc_tim_t _tim[] = {
    { TIM1, tim1_clk_enable, tim1_clk_disable, tim1_periph_reset },
    { TIM2, tim2_clk_enable, tim2_clk_disable, tim2_periph_reset },
    { TIM3, tim3_clk_enable, tim3_clk_disable, tim3_periph_reset },
    { TIM4, tim4_clk_enable, tim4_clk_disable, tim4_periph_reset },
    { TIM5, tim5_clk_enable, tim5_clk_disable, tim5_periph_reset },
    { TIM6, tim6_clk_enable, tim6_clk_disable, tim6_periph_reset },
    { TIM7, tim7_clk_enable, tim7_clk_disable, tim7_periph_reset },
    { TIM8, tim8_clk_enable, tim8_clk_disable, tim8_periph_reset },
    { TIM9, tim9_clk_enable, tim9_clk_disable, tim9_periph_reset },
    { TIM10, tim10_clk_enable, tim10_clk_disable, tim10_periph_reset },
    { TIM11, tim11_clk_enable, tim11_clk_disable, tim11_periph_reset },
    { TIM12, tim12_clk_enable, tim12_clk_disable, tim12_periph_reset },
    { TIM13, tim13_clk_enable, tim13_clk_disable, tim13_periph_reset },
    { TIM14, tim14_clk_enable, tim14_clk_disable, tim14_periph_reset },
};

void rcc_init(void)
{
    const int size = sizeof(_gpio) / sizeof(gpio_t);
    for (int i = 0; i < size; i++)
    {
        _gpio[i].reference_count = 0ul;
    }

    _gpio_mutex = xSemaphoreCreateMutexStatic(&_gpio_mutex_storage);
    assert(_gpio_mutex);
}

void rcc_deinit(void)
{
    vSemaphoreDelete(_gpio_mutex);
    _gpio_mutex = NULL;
}

void rcc_timx_clk_enable(const TIM_TypeDef *tim)
{
    assert(tim);

    const int i = find_tim_index(tim);
    assert(-1 != i);

    _tim[i].clk_enable_fn();
}

void rcc_timx_clk_disable(const TIM_TypeDef *tim)
{
    assert(tim);

    const int i = find_tim_index(tim);
    assert(-1 != i);

    _tim[i].clk_disable_fn();
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
    return HAL_OK;  //TODO: how to shut it down
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
    return HAL_OK; //TODO: how to shut it down
}

void rcc_gpiox_clk_enable(const GPIO_TypeDef *gpio)
{
    assert(gpio);

    const int i = find_gpio_index(gpio);
    assert(-1 != i);

    gpio_lock();

    if (0ul == _gpio[i].reference_count)
    {
        _gpio[i].clk_enable_fn();
    }

    _gpio[i].reference_count = _gpio[i].reference_count + 1ul;

    gpio_unlock();
}

void rcc_gpiox_clk_disable(const GPIO_TypeDef *gpio)
{
    assert(gpio);

    const int i = find_gpio_index(gpio);
    assert(-1 != i);

    gpio_lock();

    _gpio[i].reference_count = _gpio[i].reference_count - 1ul;

    if (0ul == _gpio[i].reference_count)
    {
        _gpio[i].clk_disable_fn();
    }

    gpio_unlock();
}

void rcc_usartx_clk_enable(const USART_TypeDef *usart)
{
    assert(usart);

    const int i = find_usart_index(usart);
    assert(-1 != i);

    _usart[i].clk_enable_fn();
}

void rcc_usartx_clk_disable(const USART_TypeDef *usart)
{
    assert(usart);

    const int i = find_usart_index(usart);
    assert(-1 != i);

    _usart[i].clk_disable_fn();
}

void rcc_usartx_periph_reset(const USART_TypeDef *usart)
{
    assert(usart);

    const int i = find_usart_index(usart);
    assert(-1 != i);

    _usart[i].periph_reset_fn();
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

static int find_gpio_index(const GPIO_TypeDef *gpio)
{
    int index = -1;
    const int size = sizeof(_gpio) / sizeof(gpio_t);

    for (int i = 0; i < size; i++)
    {
        if (gpio == _gpio[i].base_address)
        {
            index = i;
            break;
        }
    }

    return index;
}

static inline void gpio_lock(void)
{
    xSemaphoreTake(_gpio_mutex, portMAX_DELAY);
}

static inline void gpio_unlock(void)
{
    xSemaphoreGive(_gpio_mutex);
}


