/**
 * @ingroup    core_config
 *
 * @{
 * @file       core_config.h
 * @brief      Core configuration options
 *
 */

#ifndef __CORE_CONFIG_H__
#define __CORE_CONFIG_H__

#include <inttypes.h>
#include <limits.h>

/**
 * @brief   Architecture specific modifier used for printing sizes
 */
#if (UINT_MAX == SIZE_MAX)
#define PRI_SIZE_T_MODIFIER ""
#elif (ULONG_MAX == SIZE_MAX)
#define PRI_SIZE_T_MODIFIER "l"
#else
#error Unsupported size_t length
#endif

/**
 * @brief   Macro holding the format specifier to print an `size_t` variable
 *          in decimal representation.
 */
#define PRIuSIZE PRI_SIZE_T_MODIFIER "u"

/**
 * @brief   Select fastest bitarithm_lsb implementation
 * @{
 */
#ifdef __ARM_FEATURE_CLZ
#define BITARITHM_LSB_BUILTIN
#define BITARITHM_HAS_CLZ
#else
#define BITARITHM_LSB_LOOKUP
#endif
/** @} */

/**
 * @brief   Minimal amount of stack size that is needed to use printf in a task
 */
#define TASK_EXTRA_STACKSIZE_PRINTF    (2048)

/**
 * @brief   Format string macro for text section pointer
 */
#define PRIxTXTPTR                     PRIxPTR

/**
 * @brief   UART peripheral configuration for panic
 * @{
 */
#define PANIC_USARTx                   USART3
#define PANIC_UART_CLK_ENABLE          __HAL_RCC_USART3_CLK_ENABLE
#define PANIC_UART_FORCE_RESET         __HAL_RCC_USART3_FORCE_RESET
#define PANIC_UART_RELEASE_RESET       __HAL_RCC_USART3_RELEASE_RESET
#define PANIC_UART_TX_PIN              GPIO_PIN_10
#define PANIC_UART_TX_GPIO_PORT        GPIOB
#define PANIC_UART_TX_GPIO_CLK_ENABLE  __HAL_RCC_GPIOB_CLK_ENABLE
#define PANIC_UART_GPIO_AFx_USARTx     GPIO_AF7_USART3
/** @} */

#endif /* __CORE_CONFIG_H__ */
/** @} */

