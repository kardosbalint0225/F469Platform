/*
 * led_config.h
 *
 *  Created on: Jan 30, 2024
 *      Author: Balint
 */

#ifndef __LED_CONFIG_H__
#define __LED_CONFIG_H__

#define LED1_PIN                                    GPIO_PIN_6
#define LED1_GPIO_PORT                              GPIOG
#define LED1_PIN_GPIO_CLK_ENABLE                    __HAL_RCC_GPIOG_CLK_ENABLE
#define LED2_PIN                                    GPIO_PIN_4
#define LED2_GPIO_PORT                              GPIOD
#define LED2_PIN_GPIO_CLK_ENABLE                    __HAL_RCC_GPIOD_CLK_ENABLE
#define LED3_PIN                                    GPIO_PIN_5
#define LED3_GPIO_PORT                              GPIOD
#define LED3_PIN_GPIO_CLK_ENABLE                    __HAL_RCC_GPIOD_CLK_ENABLE
#define LED4_PIN                                    GPIO_PIN_3
#define LED4_GPIO_PORT                              GPIOK
#define LED4_PIN_GPIO_CLK_ENABLE                    __HAL_RCC_GPIOK_CLK_ENABLE

#endif /* __LED_CONFIG_H__ */
