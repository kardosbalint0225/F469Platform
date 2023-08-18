/*
 * uart_console.h
 *
 *  Created on: 2023. jul. 9.
 *      Author: Balint
 */

#ifndef _UART_CONSOLE_H_
#define _UART_CONSOLE_H_

#include <stdint.h>

enum _UART_CONSOLE_ERROR
{
    UART_CONSOLE_ERROR_TX_CPLT_SEMPHR_CREATE   = 0x00000001UL,
    UART_CONSOLE_ERROR_TX_AVAIL_QUEUE_CREATE   = 0x00000002UL,
    UART_CONSOLE_ERROR_TX_READY_QUEUE_CREATE   = 0x00000004UL,
    UART_CONSOLE_ERROR_RX_QUEUE_CREATE         = 0x00000008UL,
    UART_CONSOLE_ERROR_TX_AVAIL_QUEUE_INIT     = 0x00000010UL,
    UART_CONSOLE_ERROR_WRITE_TASK_CREATE       = 0x00000020UL,
    UART_CONSOLE_ERROR_UART_RECEIVE_IT         = 0x00000040UL,
    UART_CONSOLE_ERROR_REGISTER_MSPINIT_CB     = 0x00000080UL,
    UART_CONSOLE_ERROR_REGISTER_MSPDEINIT_CB   = 0x00000100UL,
    UART_CONSOLE_ERROR_UART_INIT               = 0x00000200UL,
    UART_CONSOLE_ERROR_REGISTER_TX_CPLT_CB     = 0x00000400UL,
    UART_CONSOLE_ERROR_REGISTER_RX_CPLT_CB     = 0x00000800UL,
    UART_CONSOLE_ERROR_DMA_INIT                = 0x00001000UL,
    UART_CONSOLE_ERROR_UNREGISTER_TX_CPLT_CB   = 0x00002000UL,
    UART_CONSOLE_ERROR_UNREGISTER_RX_CPLT_CB   = 0x00004000UL,
    UART_CONSOLE_ERROR_UART_DEINIT             = 0x00008000UL,
    UART_CONSOLE_ERROR_UNREGISTER_MSPINIT_CB   = 0x00010000UL,
    UART_CONSOLE_ERROR_UNREGISTER_MSPDEINIT_CB = 0x00020000UL,
    UART_CONSOLE_ERROR_DMA_DEINIT              = 0x00040000UL,
    UART_CONSOLE_ERROR_TX_AVAIL_QUEUE_RECEIVE  = 0x00080000UL,
    UART_CONSOLE_ERROR_TX_READY_QUEUE_SEND     = 0x00100000UL,
    UART_CONSOLE_ERROR_TX_READY_QUEUE_RECEIVE  = 0x00200000UL,
    UART_CONSOLE_ERROR_UART_TRANSMIT_DMA       = 0x00400000UL,
    UART_CONSOLE_ERROR_TX_CPLT_SEMPHR_TAKE     = 0x00800000UL,
    UART_CONSOLE_ERROR_TX_AVAIL_QUEUE_SEND     = 0x01000000UL,

    UART_CONSOLE_ERROR_UINT_MAX                = 0xFFFFFFFFUL,
};

void     uart_console_init(void);
void     uart_console_deinit(void);
int      __uart_console_write(char *buf, int len);
uint32_t uart_console_get_error(void);

#endif /* _UART_CONSOLE_H_ */

