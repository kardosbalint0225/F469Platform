/**
 * @ingroup    system_config
 *
 * @{
 * @file       sdcard_config.h
 * @brief      SD Card configuration options
 *
 */
#ifndef __SDCARD_CONFIG_H__
#define __SDCARD_CONFIG_H__

/**
 * @brief Definitions for the Card Detect Pin and the Monitor Task
 */
#define SDCARD_CD_PIN_DEBOUNCE_TIMEOUT_MS       5000ul
#define SDCARD_CD_PIN_GPIO_PORT                 GPIOG
#define SDCARD_CD_PIN                           GPIO_PIN_2
#define SDCARD_CD_PIN_EXTI_GPIO                 EXTI_GPIOG
#define SDCARD_CD_PIN_EXTI_LINE                 EXTI_LINE_2
#define SDCARD_CD_PIN_EXTIx_IRQn                EXTI2_IRQn
#define SDCARD_CD_PIN_EXTIx_IRQ_PRIORITY        10ul
#define SDCARD_CD_PIN_EXTIx_IRQHandler          EXTI2_IRQHandler
#define SDCARD_MONITOR_TASK_PRIORITY              2ul
#define SDCARD_MONITOR_TASK_STACKSIZE             (configMINIMAL_STACK_SIZE + 32 * configMINIMAL_STACK_SIZE)

/**
 * @brief Definitions for the SDIO CMD, CLK and Data ports / pins
 */
#define SDCARD_CMD_PIN_GPIO_PORT                GPIOD
#define SDCARD_CMD_PIN                          GPIO_PIN_2
#define SDCARD_CLK_PIN_GPIO_PORT                GPIOC
#define SDCARD_CLK_PIN                          GPIO_PIN_12
#define SDCARD_D3_PIN_GPIO_PORT                 GPIOC
#define SDCARD_D3_PIN                           GPIO_PIN_11
#define SDCARD_D2_PIN_GPIO_PORT                 GPIOC
#define SDCARD_D2_PIN                           GPIO_PIN_10
#define SDCARD_D1_PIN_GPIO_PORT                 GPIOC
#define SDCARD_D1_PIN                           GPIO_PIN_9
#define SDCARD_D0_PIN_GPIO_PORT                 GPIOC
#define SDCARD_D0_PIN                           GPIO_PIN_8
#define SDCARD_GPIO_AFx_SDIO                    GPIO_AF12_SDIO

/**
 * @brief Definitions for the Tx / Rx DMA Channels and interrupts
 */
#define SDCARD_SDIO_IRQ_PRIORITY                15ul
#define SDCARD_DMAx_TX_CHANNEL                  DMA_CHANNEL_4
#define SDCARD_DMAx_RX_CHANNEL                  DMA_CHANNEL_4
#define SDCARD_DMAx                             DMA2
#define SDCARD_DMAx_TX_STREAM                   DMA2_Stream6
#define SDCARD_DMAx_RX_STREAM                   DMA2_Stream3
#define SDCARD_DMAx_TX_IRQn                     DMA2_Stream6_IRQn
#define SDCARD_DMAx_RX_IRQn                     DMA2_Stream3_IRQn
#define SDCARD_DMAx_TX_IRQ_PRIORITY             15ul
#define SDCARD_DMAx_RX_IRQ_PRIORITY             15ul
#define SDCARD_DMA_BLOCK_TRANSFER_TIMEOUT_MS    5000ul
#define SDCARD_DMAx_RX_STREAM_IRQHandler        DMA2_Stream3_IRQHandler
#define SDCARD_DMAx_TX_STREAM_IRQHandler        DMA2_Stream6_IRQHandler

#endif /* __SDCARD_CONFIG_H__ */
/** @} */

