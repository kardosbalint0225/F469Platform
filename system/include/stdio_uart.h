/*
 * stdio_uart.h
 *
 *  Created on: Feb 25, 2024
 *      Author: Balint
 */

#ifndef __STDIO_UART_H__
#define __STDIO_UART_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "queue.h"

#include <stddef.h>

int stdio_uart_init(void);
int stdio_uart_deinit(void);
int stdio_uart_add_stdin_listener(const QueueHandle_t hqueue);
ssize_t stdio_uart_read(void *buffer, size_t count);
ssize_t stdio_uart_write(const void *buffer, size_t len);
ssize_t stdio_uart_write_blocking(const void *buffer, size_t len);

#ifdef __cplusplus
}
#endif
#endif /* __STDIO_UART_H__ */
