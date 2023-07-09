/*
 * uart_console.h
 *
 *  Created on: 2023. jul. 9.
 *      Author: Balint
 */

#ifndef _UART_CONSOLE_H_
#define _UART_CONSOLE_H_

void uart_console_init(void);
void uart_console_deinit(void);
int  __uart_console_write(char *buf, int len);

#endif /* _UART_CONSOLE_H_ */

