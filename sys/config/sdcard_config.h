/*
 * sdmmc_config.h
 *
 *  Created on: 2023. okt. 28.
 *      Author: Balint
 */

#ifndef _SDCARD_CONFIG_H_
#define _SDCARD_CONFIG_H_

#define SDCARD_CD_PIN_DEBOUNCE_TIMEOUT_MS       5000ul
#define SDCARD_CD_PIN_GPIO_CLK_ENABLE           __HAL_RCC_GPIOG_CLK_ENABLE
#define SDCARD_CD_PIN_GPIO_PORT                 GPIOG
#define SDCARD_CD_PIN                           GPIO_PIN_2
#define SDCARD_CD_PIN_EXTI_GPIO                 EXTI_GPIOG
#define SDCARD_CD_PIN_EXTI_LINE                 EXTI_LINE_2
#define SDCARD_CD_PIN_EXTIx_IRQn                EXTI2_IRQn
#define SDCARD_CD_PIN_EXTIx_IRQ_PRIORITY        10
#define SDCARD_MOUNT_TASK_PRIORITY              2
#define SDCARD_MOUNT_TASK_STACKSIZE             (32 * configMINIMAL_STACK_SIZE)

#define SDCARD_CMD_PIN_GPIO_CLK_ENABLE          __HAL_RCC_GPIOD_CLK_ENABLE
#define SDCARD_CMD_PIN_GPIO_PORT                GPIOD
#define SDCARD_CMD_PIN                          GPIO_PIN_2
#define SDCARD_CLK_PIN_GPIO_CLK_ENABLE          __HAL_RCC_GPIOC_CLK_ENABLE
#define SDCARD_CLK_PIN_GPIO_PORT                GPIOC
#define SDCARD_CLK_PIN                          GPIO_PIN_12
#define SDCARD_D3_PIN_GPIO_CLK_ENABLE           __HAL_RCC_GPIOC_CLK_ENABLE
#define SDCARD_D3_PIN_GPIO_PORT                 GPIOC
#define SDCARD_D3_PIN                           GPIO_PIN_11
#define SDCARD_D2_PIN_GPIO_CLK_ENABLE           __HAL_RCC_GPIOC_CLK_ENABLE
#define SDCARD_D2_PIN_GPIO_PORT                 GPIOC
#define SDCARD_D2_PIN                           GPIO_PIN_10
#define SDCARD_D1_PIN_GPIO_CLK_ENABLE           __HAL_RCC_GPIOC_CLK_ENABLE
#define SDCARD_D1_PIN_GPIO_PORT                 GPIOC
#define SDCARD_D1_PIN                           GPIO_PIN_9
#define SDCARD_D0_PIN_GPIO_CLK_ENABLE           __HAL_RCC_GPIOC_CLK_ENABLE
#define SDCARD_D0_PIN_GPIO_PORT                 GPIOC
#define SDCARD_D0_PIN                           GPIO_PIN_8
#define SDCARD_GPIO_AFx_SDIO                    GPIO_AF12_SDIO

#define SDCARD_SDIO_IRQ_PRIORITY                15
#define SDCARD_DMAx_CLK_ENABLE                  __DMA2_CLK_ENABLE
#define SDCARD_DMAx_TX_CHANNEL                  DMA_CHANNEL_4
#define SDCARD_DMAx_RX_CHANNEL                  DMA_CHANNEL_4
#define SDCARD_DMAx_TX_STREAM                   DMA2_Stream6
#define SDCARD_DMAx_RX_STREAM                   DMA2_Stream3
#define SDCARD_DMAx_TX_IRQn                     DMA2_Stream6_IRQn
#define SDCARD_DMAx_RX_IRQn                     DMA2_Stream3_IRQn
#define SDCARD_DMAx_TX_IRQ_PRIORITY             15
#define SDCARD_DMAx_RX_IRQ_PRIORITY             15
#define SDCARD_DMA_BLOCK_TRANSFER_TIMEOUT_MS    50

#endif /* _SDCARD_CONFIG_H_ */
