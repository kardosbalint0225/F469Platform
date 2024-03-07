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

/**
 * @brief  Initializes the STDIO_UART peripheral
 *
 * @param  None
 *
 * @return  0 on success
 * @return < 0 on error
 *
 * @note   The communication is configured 115200 Baudrate 8N1 with no
 *         flow-control.
 */
int stdio_uart_init(void);

/**
 * @brief  De-initializes the STDIO_UART peripheral
 *
 * @param  None
 *
 * @return  0 on success
 * @return < 0 on error
 */
int stdio_uart_deinit(void);

/**
 * @brief Adds the given queue handle to the listeners list
 *
 * @param[in] hqueue QueueHandle_t handle to be added to the listeners list
 *
 * @return 0 on success
 * @return < 0 on error
 *
 * @note The CLI adds its input queue handle to the listeners list to
 *       receive the characters when the stdin is not used by other tasks
 *       If a task uses a function which relies on stdin then the received bytes
 *       over UART are passed to the stdin queue bypassing the CLI input queue
 */
int stdio_uart_add_stdin_listener(const QueueHandle_t hqueue);

/**
 * @brief read @p len bytes from stdio uart into @p buffer
 *
 * @param[out]  buffer  buffer to read into
 * @param[in]   max_len nr of bytes to read
 *
 * @return nr of bytes read
 * @return <0 on error
 */
ssize_t stdio_uart_read(void *buffer, size_t max_len);

/**
 * @brief write @p len bytes from @p buffer to stdio uart
 *
 * @param[in]   buffer  buffer to read from
 * @param[in]   len     nr of bytes to write
 *
 * @return nr of bytes written
 * @return <0 on error
 */
ssize_t stdio_uart_write(const void *buffer, size_t len);

/**
 * @brief write @p len bytes from @p buffer to stdio uart in blocking mode
 *
 * @param[in]   buffer  buffer to read from
 * @param[in]   len     nr of bytes to write
 *
 * @return nr of bytes written
 * @return <0 on error
 *
 * @note A call to this function will abort any ongoing transmit / receive
 *       operations
 *       This function exists only to support panic() and assert() functions
 *       and should not be used in any other cases
 */
ssize_t stdio_uart_write_blocking(const void *buffer, size_t len);

#ifdef __cplusplus
}
#endif
#endif /* __STDIO_UART_H__ */
