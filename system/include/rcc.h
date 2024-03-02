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

void rcc_init(void);
void rcc_deinit(void);

HAL_StatusTypeDef rtc_clock_source_init(void);
HAL_StatusTypeDef rtc_clock_source_deinit(void);
HAL_StatusTypeDef clk48_clock_init(void);
HAL_StatusTypeDef clk48_clock_deinit(void);
HAL_StatusTypeDef sdio_clock_source_init(void);
HAL_StatusTypeDef sdio_clock_source_deinit(void);

void rcc_gpiox_clk_enable(const GPIO_TypeDef *gpio);
void rcc_gpiox_clk_disable(const GPIO_TypeDef *gpio);

void rcc_timx_clk_enable(const TIM_TypeDef *tim);
void rcc_timx_clk_disable(const TIM_TypeDef *tim);
void rcc_timx_periph_reset(const TIM_TypeDef *tim);

void rcc_usartx_clk_enable(const USART_TypeDef *usart);
void rcc_usartx_clk_disable(const USART_TypeDef *usart);
void rcc_usartx_periph_reset(const USART_TypeDef *usart);

#ifdef __cplusplus
}
#endif
#endif /* __RCC_H__ */
