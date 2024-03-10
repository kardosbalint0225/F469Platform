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

int rcc_system_clock_init(void);
int rcc_system_clock_bus_clocks_init(void);
int rcc_system_clock_external_oscillators_init(void);
int rcc_internal_oscillators_disable(void);

HAL_StatusTypeDef rtc_clock_source_init(void);
HAL_StatusTypeDef rtc_clock_source_deinit(void);
HAL_StatusTypeDef clk48_clock_init(void);
HAL_StatusTypeDef clk48_clock_deinit(void);
HAL_StatusTypeDef sdio_clock_source_init(void);
HAL_StatusTypeDef sdio_clock_source_deinit(void);

void rcc_periph_clk_enable(const void *periph);
void rcc_periph_clk_disable(const void *periph);
void rcc_periph_reset(const void *periph);

#ifdef __cplusplus
}
#endif
#endif /* __RCC_H__ */
