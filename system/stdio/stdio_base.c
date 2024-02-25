/*
 * stdio_base.c
 *
 *  Created on: Feb 25, 2024
 *      Author: Balint
 */
#include "stdio_base.h"
#include "stdio_uart.h"

#include <assert.h>

void stdio_init(void)
{
    int ret = stdio_uart_init();
    assert(0 == ret);
}

void stdio_deinit(void)
{
    int ret = stdio_uart_deinit();
    assert(0 == ret);
}

int stdio_add_stdin_listener(const QueueHandle_t hqueue)
{
    int ret = stdio_uart_add_stdin_listener(hqueue);
    return ret;
}

#if IS_USED(MODULE_STDIO_AVAILABLE)
int stdio_available(void)
{
    return -ENOTSUP;
}
#endif

ssize_t stdio_read(void *buffer, size_t count)
{
    ssize_t ret = stdio_uart_read(buffer, count);
    return ret;
}

ssize_t stdio_write(const void *buffer, size_t len)
{
    ssize_t ret = stdio_uart_write(buffer, len);
    return ret;
}

ssize_t stdio_write_blocking(const void* buffer, size_t len)
{
    ssize_t ret = stdio_uart_write_blocking(buffer, len);
    return ret;
}
