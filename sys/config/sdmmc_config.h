/*
 * sdmmc_config.h
 *
 *  Created on: 2023. okt. 28.
 *      Author: Balint
 */

#ifndef _SDMMC_CONFIG_H_
#define _SDMMC_CONFIG_H_

#define SDMMC_CARD_DETECT_DEBOUNCE_TIMEOUT_MS    5000U
#define SDMMC_CARD_DETECT_GPIO_PORT              GPIOG
#define SDMMC_CARD_DETECT_GPIO_PIN               GPIO_PIN_2
#define SDMMC_CARD_DETECT_GPIO_CLK_ENABLE        __HAL_RCC_GPIOG_CLK_ENABLE
#define SDMMC_CARD_DETECT_EXTI_GPIO              EXTI_GPIOG
#define SDMMC_CARD_DETECT_EXTI_LINE              EXTI_LINE_2
#define SDMMC_CARD_DETECT_EXTIx_IRQn             EXTI2_IRQn
#define SDMMC_CARD_DETECT_EXTIx_IRQ_PRIORITY     10
#define SDMMC_CARD_MOUNT_TASK_PRIORITY           2
#define SDMMC_CARD_MOUNT_TASK_STACKSIZE          (4 * configMINIMAL_STACK_SIZE)

#endif /* _SDMMC_CONFIG_H_ */
