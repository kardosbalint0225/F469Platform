/**
 * MIT License
 *
 * Copyright (c) 2024 Balint Kardos
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/**
 * @ingroup     core
 *
 * @file        rcc.h
 * @brief       System and Peripheral Clock Management
 */

#ifndef __RCC_H__
#define __RCC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

/**
 * @brief Initializes the RCC (Reset and Clock Control) module.
 *
 * This function initializes the reference counts for the given peripheral
 * clocks to 0 and creates a mutex for the reference count access protection
 */
void rcc_init(void);

/**
 * @brief De-initializes the RCC (Reset and Clock Control) module.
 */
void rcc_deinit(void);

/**
 * @brief Initializes the clock system.
 *
 * This function initializes the clock system by configuring the main internal
 * regulator output voltage, initializing external oscillators, initializing
 * bus clocks, and disabling internal oscillators.
 *
 * @return 0 on success
 * @return < 0 on error
 */
int rcc_clock_system_init(void);

/**
 * @brief Configures the bus clocks.
 *
 * This function configures the HCLK, SYSCLK, PCLK1, PCLK2
 * clock sources and dividers.
 *
 * @return 0 on success
 * @return < 0 on error
 */
int rcc_bus_clocks_config(void);

/**
 * @brief Initializes the external oscillators
 *
 * This function initializes the HSE and LSE external oscillators.
 * It configures the PLL to use the HSE as its source and enables OverDrive mode
 * to run at 180 MHz.
 *
 * @return 0 on success
 * @return < 0 on error
 */
int rcc_external_oscillators_init(void);

/**
 * @brief Disables the internal oscillators
 *
 * This function disables the HSI and LSI oscillators.
 *
 * @return 0 on success
 * @return < 0 on error
 */
int rcc_internal_oscillators_disable(void);

/**
 * @brief     Enables the clock for a peripheral.
 *
 * This function enables the clock for the specified peripheral. It checks
 * if the peripheral's reference count is zero, and if so, calls the
 * clock enable function to enable the peripheral clock. It then increments
 * the reference count for the peripheral.
 *
 * @param[in] periph Pointer to the peripheral to enable the clock for.
 */
void rcc_periph_clk_enable(const void *periph);

/**
 * @brief     Disables the clock for a peripheral.
 *
 * This function disables the clock for the specified peripheral. It decrements
 * the reference count for the peripheral, and if the reference count becomes
 * zero, calls the clock disable function to disable the peripheral clock.
 *
 * @param[in] periph Pointer to the peripheral to disable the clock for.
 */
void rcc_periph_clk_disable(const void *periph);

/**
 * @brief     Resets a peripheral.
 *
 * This function resets the specified peripheral.
 *
 * @param[in] periph Pointer to the peripheral to reset.
 */
void rcc_periph_reset(const void *periph);

/**
 * @brief Initializes the RTC clock source.
 *
 * This function configures the LSE oscillator as the RTC clock source.
 *
 * @return HAL status
 */
HAL_StatusTypeDef rtc_clock_source_init(void);

/**
 * @brief De-initializes the RTC clock source.
 *
 * This function disconnects the LSE from the RTC clock source.
 *
 * @return HAL status
 */
HAL_StatusTypeDef rtc_clock_source_deinit(void);

/**
 * @brief Initializes the 48 MHz clock.
 *
 * This function configures the 48 MHz clock to be used by the SDIO and USB FS Host.
 * PLLSAIP is selected as the clock source. Subsequent calls to this function will not reconfigure
 * the 48 MHz clock but will increment the reference count for it.
 *
 * @return HAL status
 */
HAL_StatusTypeDef clk48_clock_init(void);

/**
 * @brief De-initializes the 48 MHz clock.
 *
 * TODO: How to shut down
 *
 * @return HAL status
 */
HAL_StatusTypeDef clk48_clock_deinit(void);

/**
 * @brief Initializes the clock for SDIO.
 *
 * This function initializes the clock for the SDIO peripheral. It initializes
 * the 48 MHz clock required for SDIO operation. If the 48 MHz clock initialization
 * is successful, it configures the SDIO peripheral clock source to use the 48 MHz clock.
 *
 * @return HAL status
 */
HAL_StatusTypeDef sdio_clock_source_init(void);

/**
 * @brief De-initializes the clock for SDIO.
 *
 * TODO: How to shut down
 *
 * @return HAL status
 */
HAL_StatusTypeDef sdio_clock_source_deinit(void);

#ifdef __cplusplus
}
#endif
#endif /* __RCC_H__ */
