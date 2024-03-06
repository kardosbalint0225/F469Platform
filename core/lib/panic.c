/*
 * panic.c
 *
 *  Created on: Feb 22, 2024
 *      Author: Balint
 */
#include "panic.h"
#include "debug.h"
#include "gpio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stm32f4xx_hal.h"
#include <stdarg.h>
#include <stdio.h>
#include "stdio_base.h"

static char _tx_buffer[513];

__NORETURN void panic(const char *message, ...)
{
    vTaskSuspendAll();

    va_list va;
    va_start(va, message);
    size_t len = vsnprintf(_tx_buffer, sizeof(_tx_buffer), message, va);
    va_end(va);
    stdio_write_blocking(_tx_buffer, len);

#ifdef DEBUG_ASSERT_BREAKPOINT
    DEBUG_BREAKPOINT(1);
#endif

    while (1)
    {
        led3_toggle();
        HAL_Delay(250ul);
    }
}

