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

HAL_StatusTypeDef rtc_clock_source_init(void);
HAL_StatusTypeDef rtc_clock_source_deinit(void);
HAL_StatusTypeDef clk48_clock_init(void);
HAL_StatusTypeDef clk48_clock_deinit(void);
HAL_StatusTypeDef sdio_clock_source_init(void);
HAL_StatusTypeDef sdio_clock_source_deinit(void);

void rcc_gpioa_clk_enable(void);
void rcc_gpiob_clk_enable(void);
void rcc_gpioc_clk_enable(void);
void rcc_gpiod_clk_enable(void);
void rcc_gpioe_clk_enable(void);
void rcc_gpiof_clk_enable(void);
void rcc_gpiog_clk_enable(void);
void rcc_gpioh_clk_enable(void);
void rcc_gpioi_clk_enable(void);
void rcc_gpioj_clk_enable(void);
void rcc_gpiok_clk_enable(void);
void rcc_gpioa_clk_disable(void);
void rcc_gpiob_clk_disable(void);
void rcc_gpioc_clk_disable(void);
void rcc_gpiod_clk_disable(void);
void rcc_gpioe_clk_disable(void);
void rcc_gpiof_clk_disable(void);
void rcc_gpiog_clk_disable(void);
void rcc_gpioh_clk_disable(void);
void rcc_gpioi_clk_disable(void);
void rcc_gpioj_clk_disable(void);
void rcc_gpiok_clk_disable(void);

void rcc_timx_clk_enable(const TIM_TypeDef *tim);
void rcc_timx_clk_disable(const TIM_TypeDef *tim);
void rcc_timx_periph_reset(const TIM_TypeDef *tim);

void rcc_tim1_clk_enable(void);
void rcc_tim2_clk_enable(void);
void rcc_tim3_clk_enable(void);
void rcc_tim4_clk_enable(void);
void rcc_tim5_clk_enable(void);
void rcc_tim6_clk_enable(void);
void rcc_tim7_clk_enable(void);
void rcc_tim8_clk_enable(void);
void rcc_tim9_clk_enable(void);
void rcc_tim10_clk_enable(void);
void rcc_tim11_clk_enable(void);
void rcc_tim12_clk_enable(void);
void rcc_tim13_clk_enable(void);
void rcc_tim14_clk_enable(void);
void rcc_tim1_clk_disable(void);
void rcc_tim2_clk_disable(void);
void rcc_tim3_clk_disable(void);
void rcc_tim4_clk_disable(void);
void rcc_tim5_clk_disable(void);
void rcc_tim6_clk_disable(void);
void rcc_tim7_clk_disable(void);
void rcc_tim8_clk_disable(void);
void rcc_tim9_clk_disable(void);
void rcc_tim10_clk_disable(void);
void rcc_tim11_clk_disable(void);
void rcc_tim12_clk_disable(void);
void rcc_tim13_clk_disable(void);
void rcc_tim14_clk_disable(void);
void rcc_tim1_periph_reset(void);
void rcc_tim2_periph_reset(void);
void rcc_tim3_periph_reset(void);
void rcc_tim4_periph_reset(void);
void rcc_tim5_periph_reset(void);
void rcc_tim6_periph_reset(void);
void rcc_tim7_periph_reset(void);
void rcc_tim8_periph_reset(void);
void rcc_tim9_periph_reset(void);
void rcc_tim10_periph_reset(void);
void rcc_tim11_periph_reset(void);
void rcc_tim12_periph_reset(void);
void rcc_tim13_periph_reset(void);
void rcc_tim14_periph_reset(void);

void rcc_usartx_clk_enable(const USART_TypeDef *usart);
void rcc_usartx_clk_disable(const USART_TypeDef *usart);
void rcc_usartx_periph_reset(const USART_TypeDef *usart);

void rcc_usart1_clk_enable(void);
void rcc_usart2_clk_enable(void);
void rcc_usart3_clk_enable(void);
void rcc_uart4_clk_enable(void);
void rcc_uart5_clk_enable(void);
void rcc_usart6_clk_enable(void);
void rcc_uart7_clk_enable(void);
void rcc_uart8_clk_enable(void);
void rcc_usart1_clk_disable(void);
void rcc_usart2_clk_disable(void);
void rcc_usart3_clk_disable(void);
void rcc_uart4_clk_disable(void);
void rcc_uart5_clk_disable(void);
void rcc_usart6_clk_disable(void);
void rcc_uart7_clk_disable(void);
void rcc_uart8_clk_disable(void);

void rcc_usart1_periph_reset(void);
void rcc_usart2_periph_reset(void);
void rcc_usart3_periph_reset(void);
void rcc_uart4_periph_reset(void);
void rcc_uart5_periph_reset(void);
void rcc_usart6_periph_reset(void);
void rcc_uart7_periph_reset(void);
void rcc_uart8_periph_reset(void);

#ifdef __cplusplus
}
#endif
#endif /* __RCC_H__ */
