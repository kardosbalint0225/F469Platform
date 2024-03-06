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

static int find_periph_index(const void *periph);
static inline void periph_lock(void);
static inline void periph_unlock(void);

static SemaphoreHandle_t _periph_mutex = NULL;
static StaticSemaphore_t _periph_mutex_storage;

typedef struct {
    const void * const base_address;
    const void (* const clk_enable_fn)(void);
    const void (* const clk_disable_fn)(void);
    const void (* const periph_reset_fn)(void);
    uint32_t reference_count;
} rcc_periph_t;

static rcc_periph_t _periph[] = {
    { (const void *)GPIOA, gpioa_clk_enable, gpioa_clk_disable },
    { (const void *)GPIOB, gpiob_clk_enable, gpiob_clk_disable },
    { (const void *)GPIOC, gpioc_clk_enable, gpioc_clk_disable },
    { (const void *)GPIOD, gpiod_clk_enable, gpiod_clk_disable },
    { (const void *)GPIOE, gpioe_clk_enable, gpioe_clk_disable },
    { (const void *)GPIOF, gpiof_clk_enable, gpiof_clk_disable },
    { (const void *)GPIOG, gpiog_clk_enable, gpiog_clk_disable },
    { (const void *)GPIOH, gpioh_clk_enable, gpioh_clk_disable },
    { (const void *)GPIOI, gpioi_clk_enable, gpioi_clk_disable },
    { (const void *)GPIOJ, gpioj_clk_enable, gpioj_clk_disable },
    { (const void *)GPIOK, gpiok_clk_enable, gpiok_clk_disable },

    { (const void *)USART1, usart1_clk_enable, usart1_clk_disable, usart1_periph_reset },
    { (const void *)USART2, usart2_clk_enable, usart2_clk_disable, usart2_periph_reset },
    { (const void *)USART3, usart3_clk_enable, usart3_clk_disable, usart3_periph_reset },
    { (const void *)UART4, uart4_clk_enable, uart4_clk_disable, uart4_periph_reset },
    { (const void *)UART5, uart5_clk_enable, uart5_clk_disable, uart5_periph_reset},
    { (const void *)USART6, usart6_clk_enable, usart6_clk_disable, usart6_periph_reset },
    { (const void *)UART7, uart7_clk_enable, uart7_clk_disable, uart7_periph_reset },
    { (const void *)UART8, uart8_clk_enable, uart8_clk_disable, uart8_periph_reset },

    { (const void *)TIM1, tim1_clk_enable, tim1_clk_disable, tim1_periph_reset },
    { (const void *)TIM2, tim2_clk_enable, tim2_clk_disable, tim2_periph_reset },
    { (const void *)TIM3, tim3_clk_enable, tim3_clk_disable, tim3_periph_reset },
    { (const void *)TIM4, tim4_clk_enable, tim4_clk_disable, tim4_periph_reset },
    { (const void *)TIM5, tim5_clk_enable, tim5_clk_disable, tim5_periph_reset },
    { (const void *)TIM6, tim6_clk_enable, tim6_clk_disable, tim6_periph_reset },
    { (const void *)TIM7, tim7_clk_enable, tim7_clk_disable, tim7_periph_reset },
    { (const void *)TIM8, tim8_clk_enable, tim8_clk_disable, tim8_periph_reset },
    { (const void *)TIM9, tim9_clk_enable, tim9_clk_disable, tim9_periph_reset },
    { (const void *)TIM10, tim10_clk_enable, tim10_clk_disable, tim10_periph_reset },
    { (const void *)TIM11, tim11_clk_enable, tim11_clk_disable, tim11_periph_reset },
    { (const void *)TIM12, tim12_clk_enable, tim12_clk_disable, tim12_periph_reset },
    { (const void *)TIM13, tim13_clk_enable, tim13_clk_disable, tim13_periph_reset },
    { (const void *)TIM14, tim14_clk_enable, tim14_clk_disable, tim14_periph_reset },
};

void rcc_init(void)
{
    const int size = sizeof(_periph) / sizeof(rcc_periph_t);
    for (int i = 0; i < size; i++)
    {
        _periph[i].reference_count = 0ul;
    }

    _periph_mutex = xSemaphoreCreateMutexStatic(&_periph_mutex_storage);
    assert(_periph_mutex);
}

void rcc_deinit(void)
{
    vSemaphoreDelete(_periph_mutex);
    _periph_mutex = NULL;
}

void rcc_periph_clk_enable(const void *periph)
{
    assert(periph);

    const int i = find_periph_index(periph);
    assert(-1 != i);
    assert(_periph[i].clk_enable_fn);

    periph_lock();

    if (0ul == _periph[i].reference_count)
    {
        _periph[i].clk_enable_fn();
    }

    _periph[i].reference_count = _periph[i].reference_count + 1ul;

    periph_unlock();
}

void rcc_periph_clk_disable(const void *periph)
{
    assert(periph);

    const int i = find_periph_index(periph);
    assert(-1 != i);
    assert(_periph[i].clk_disable_fn);

    periph_lock();

    _periph[i].reference_count = _periph[i].reference_count - 1ul;

    if (0ul == _periph[i].reference_count)
    {
        _periph[i].clk_disable_fn();
    }

    periph_unlock();
}

void rcc_periph_reset(const void *periph)
{
    assert(periph);

    const int i = find_periph_index(periph);
    assert(-1 != i);
    assert(_periph[i].periph_reset_fn);

    _periph[i].periph_reset_fn();
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

static int find_periph_index(const void *periph)
{
    int index = -1;
    const int size = sizeof(_periph) / sizeof(rcc_periph_t);

    for (int i = 0; i < size; i++)
    {
        if (periph == _periph[i].base_address)
        {
            index = i;
            break;
        }
    }

    return index;
}

static inline void periph_lock(void)
{
    xSemaphoreTake(_periph_mutex, portMAX_DELAY);
}

static inline void periph_unlock(void)
{
    xSemaphoreGive(_periph_mutex);
}


