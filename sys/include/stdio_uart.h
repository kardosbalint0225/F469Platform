/*
 * Copyright (C) 2015 Kaspar Schleiser <kaspar@schleiser.de>
 *               2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_stdio_uart STDIO over UART
 * @ingroup     sys_stdio
 *
 * @brief       Standard input/output backend using UART
 *
 * To enable stdio over UART, enable the `stdio_uart` module:
 *
 *     USEMODULE += stdio_uart
 *
 * @note    For many board, `stdio_uart` is already the default stdio backend
 *          and therefore already enabled.
 *
 * ## Input
 *
 * @warning Standard input is disabled by default on UART. To enable it, load
 *          the `stdin` module in your application:
 * ```
 * USEMODULE += stdin
 * ```
 *
 * ## UART Configuration
 *
 * @note    Running `make BOARD=<your_board> term` will launch `pyterm` with the
 *          correct parameters, so mostly you do not really need to care about
 *          the UART configuration.
 *
 * By convention RIOT boards used 8N1 encoding with a symbol rate of 115200 Bd
 * for the UART used as stdio. However, some boards may have a different
 * configuration due to hardware limitations. Most notably, many AVR boards use
 * 9600 Bd as symbol rate instead, as they otherwise frequently loose an input
 * character due to losing interrupts.
 *
 * By default UNIX style line endings (`\n`) are used. However, some terminal
 * programs default to DOS style line endings (`\r\n`). It usually is better to
 * configure the terminal program on the host to use UNIX style line endings.
 * In scenarios this is not possible/desired, you can enable the (pseudo-)
 * module @ref sys_stdio_uart_onlcr to emit DOS style line endings instead.
 *
 * RIOT's shell happily accepts both DOS and UNIX style line endings in any
 * case, so typically no line ending conversion is needed on the input.
 *
 * ## STDIO from ISR
 *
 * @attention   Using STDIO over UART from interrupt context should be avoided,
 *              except for debugging purposes
 *
 * For testing purposes and using STDIO within an ISR should mostly work good
 * enough and for some platforms even reliable. Production code however should
 * fully avoid any access to STDIO from interrupt context. Instead, e.g. an
 * event could be posted to an @ref sys_event and the actual STDIO operation
 * being deferred to thread context.
 *
 * Some reasons why STDIO over UART from ISR should be avoided:
 * 1. UART is *slow* and the system easily remains in interrupt context for
 *    unacceptable long time.
 *     - E.g. sending 100 chars at 9600 baud will block the system for
 *       100 milliseconds.
 *     - Missed deadlines, lost interrupts, or watch dog timer resets can easily
 *       be caused by this.
 * 2. Even if DMA is used for UART, using STDIO within ISR can cause significant
 *    delays: If the buffer is full, an UART implementation will be forced to
 *    resort to synchronously send the data, rather than using DMA. This might
 *    cause even more headache, as the available memory in the DMA buffer when
 *    an ISR is triggered has to be assumed as randomly distributed. Thus,
 *    hard to reproduce and indeterministic bugs can be the result.
 * 3. If an ISR is triggered from a power saving mode, some peripherals or
 *    clock domains might still be offline during that ISR; including the UART
 *    peripheral. This is a valid implementation choice to allow time critical
 *    low power scenarios being covered by RIOT. Thus, be prepared to
 *    loose output when using STDIO from ISR.
 *
 * @{
 * @file
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef STDIO_UART_H
#define STDIO_UART_H

#include <stdint.h>
#include "stdio_base.h"

#ifdef __cplusplus
extern "C" {
#endif

enum _STDIO_UART_ERROR
{
    STDIO_UART_ERROR_TX_CPLT_SEMPHR_CREATE   = 0x00000001UL,
    STDIO_UART_ERROR_TX_AVAIL_QUEUE_CREATE   = 0x00000002UL,
    STDIO_UART_ERROR_TX_READY_QUEUE_CREATE   = 0x00000004UL,
    STDIO_UART_ERROR_RX_QUEUE_CREATE         = 0x00000008UL,
    STDIO_UART_ERROR_TX_AVAIL_QUEUE_INIT     = 0x00000010UL,
    STDIO_UART_ERROR_WRITE_TASK_CREATE       = 0x00000020UL,
    STDIO_UART_ERROR_UART_RECEIVE_IT         = 0x00000040UL,
    STDIO_UART_ERROR_REGISTER_MSPINIT_CB     = 0x00000080UL,
    STDIO_UART_ERROR_REGISTER_MSPDEINIT_CB   = 0x00000100UL,
    STDIO_UART_ERROR_UART_INIT               = 0x00000200UL,
    STDIO_UART_ERROR_REGISTER_TX_CPLT_CB     = 0x00000400UL,
    STDIO_UART_ERROR_REGISTER_RX_CPLT_CB     = 0x00000800UL,
    STDIO_UART_ERROR_DMA_INIT                = 0x00001000UL,
    STDIO_UART_ERROR_UNREGISTER_TX_CPLT_CB   = 0x00002000UL,
    STDIO_UART_ERROR_UNREGISTER_RX_CPLT_CB   = 0x00004000UL,
    STDIO_UART_ERROR_UART_DEINIT             = 0x00008000UL,
    STDIO_UART_ERROR_UNREGISTER_MSPINIT_CB   = 0x00010000UL,
    STDIO_UART_ERROR_UNREGISTER_MSPDEINIT_CB = 0x00020000UL,
    STDIO_UART_ERROR_DMA_DEINIT              = 0x00040000UL,
    STDIO_UART_ERROR_TX_AVAIL_QUEUE_RECEIVE  = 0x00080000UL,
    STDIO_UART_ERROR_TX_READY_QUEUE_SEND     = 0x00100000UL,
    STDIO_UART_ERROR_TX_READY_QUEUE_RECEIVE  = 0x00200000UL,
    STDIO_UART_ERROR_UART_TRANSMIT_DMA       = 0x00400000UL,
    STDIO_UART_ERROR_TX_CPLT_SEMPHR_TAKE     = 0x00800000UL,
    STDIO_UART_ERROR_TX_AVAIL_QUEUE_SEND     = 0x01000000UL,

    STDIO_UART_ERROR_UINT_MAX                = 0xFFFFFFFFUL,
};

uint32_t stdio_uart_get_error(void);

#ifdef __cplusplus
}
#endif
/** @} */
#endif /* STDIO_UART_H */
