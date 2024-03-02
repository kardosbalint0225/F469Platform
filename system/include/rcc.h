/*
 * rcc.h
 *
 *  Created on: Feb 25, 2024
 *      Author: Balint
 */

#ifndef __RCC_H__
#define __RCC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

typedef enum {
    GPIO_CLK_ENABLE = 0ul,
    GPIO_CLK_DISABLE,
} clk_state_t;

HAL_StatusTypeDef rtc_clock_source_init(void);
HAL_StatusTypeDef rtc_clock_source_deinit(void);
HAL_StatusTypeDef clk48_clock_init(void);
HAL_StatusTypeDef clk48_clock_deinit(void);
HAL_StatusTypeDef sdio_clock_source_init(void);
HAL_StatusTypeDef sdio_clock_source_deinit(void);

void gpio_a_clk_enable_disable(const clk_state_t state);
void gpio_b_clk_enable_disable(const clk_state_t state);
void gpio_c_clk_enable_disable(const clk_state_t state);
void gpio_d_clk_enable_disable(const clk_state_t state);
void gpio_e_clk_enable_disable(const clk_state_t state);
void gpio_f_clk_enable_disable(const clk_state_t state);
void gpio_g_clk_enable_disable(const clk_state_t state);
void gpio_h_clk_enable_disable(const clk_state_t state);
void gpio_i_clk_enable_disable(const clk_state_t state);
void gpio_j_clk_enable_disable(const clk_state_t state);
void gpio_k_clk_enable_disable(const clk_state_t state);

#ifdef __cplusplus
}
#endif
#endif /* __RCC_H__ */
