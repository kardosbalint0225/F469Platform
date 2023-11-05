/*
 * sdmmc_config.h
 *
 *  Created on: 2023. okt. 28.
 *      Author: Balint
 */

#ifndef _SDMMC_CONFIG_H_
#define _SDMMC_CONFIG_H_

#define SDMMC_CD_PIN_DEBOUNCE_TIMEOUT_MS    5000U
#define SDMMC_CD_PIN_GPIO_CLK_ENABLE        __HAL_RCC_GPIOG_CLK_ENABLE
#define SDMMC_CD_PIN_GPIO_PORT              GPIOG
#define SDMMC_CD_PIN                        GPIO_PIN_2
#define SDMMC_CD_PIN_EXTI_GPIO              EXTI_GPIOG
#define SDMMC_CD_PIN_EXTI_LINE              EXTI_LINE_2
#define SDMMC_CD_PIN_EXTIx_IRQn             EXTI2_IRQn
#define SDMMC_CD_PIN_EXTIx_IRQ_PRIORITY     10
#define SDMMC_MOUNT_TASK_PRIORITY           2
#define SDMMC_MOUNT_TASK_STACKSIZE          (4 * configMINIMAL_STACK_SIZE)

#define SDMMC_CMD_PIN_GPIO_CLK_ENABLE       __HAL_RCC_GPIOD_CLK_ENABLE
#define SDMMC_CMD_PIN_GPIO_PORT             GPIOD
#define SDMMC_CMD_PIN                       GPIO_PIN_2
#define SDMMC_CLK_PIN_GPIO_CLK_ENABLE       __HAL_RCC_GPIOC_CLK_ENABLE
#define SDMMC_CLK_PIN_GPIO_PORT             GPIOC
#define SDMMC_CLK_PIN                       GPIO_PIN_12
#define SDMMC_D3_PIN_GPIO_CLK_ENABLE        __HAL_RCC_GPIOC_CLK_ENABLE
#define SDMMC_D3_PIN_GPIO_PORT              GPIOC
#define SDMMC_D3_PIN                        GPIO_PIN_11
#define SDMMC_D2_PIN_GPIO_CLK_ENABLE        __HAL_RCC_GPIOC_CLK_ENABLE
#define SDMMC_D2_PIN_GPIO_PORT              GPIOC
#define SDMMC_D2_PIN                        GPIO_PIN_10
#define SDMMC_D1_PIN_GPIO_CLK_ENABLE        __HAL_RCC_GPIOC_CLK_ENABLE
#define SDMMC_D1_PIN_GPIO_PORT              GPIOC
#define SDMMC_D1_PIN                        GPIO_PIN_9
#define SDMMC_D0_PIN_GPIO_CLK_ENABLE        __HAL_RCC_GPIOC_CLK_ENABLE
#define SDMMC_D0_PIN_GPIO_PORT              GPIOC
#define SDMMC_D0_PIN                        GPIO_PIN_8
#define SDMMC_GPIO_AFx_SDIO                 GPIO_AF12_SDIO

#define SDMMC_IRQ_PRIORITY                  15
#define SDMMC_DMAx_CLK_ENABLE               __DMA2_CLK_ENABLE
#define SDMMC_DMAx_TX_CHANNEL               DMA_CHANNEL_4
#define SDMMC_DMAx_RX_CHANNEL               DMA_CHANNEL_4
#define SDMMC_DMAx_TX_STREAM                DMA2_Stream6
#define SDMMC_DMAx_RX_STREAM                DMA2_Stream3
#define SDMMC_DMAx_TX_IRQn                  DMA2_Stream6_IRQn
#define SDMMC_DMAx_RX_IRQn                  DMA2_Stream3_IRQn
#define SDMMC_DMAx_TX_IRQ_PRIORITY          15
#define SDMMC_DMAx_RX_IRQ_PRIORITY          15

#endif /* _SDMMC_CONFIG_H_ */
