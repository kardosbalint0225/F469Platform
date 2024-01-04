/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @author  Martine Lenders <mlenders@inf.fu-berlin.de>
 */
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "assert.h"
#include "cpu_conf.h"
#include "debug.h"
#include "modules.h"
#include "compiler_hints.h"

#include "stm32f4xx_hal.h"
#include "stdio_uart_config.h"
#include "gpio.h"

static char _uart_tx_buffer[513];

__NORETURN void _assert_failure(const char *file, unsigned line)
{
    HAL_NVIC_DisableIRQ(STDIO_UART_USARTx_IRQn);
    HAL_NVIC_DisableIRQ(STDIO_UART_DMAx_STREAMx_IRQn);

    STDIO_UART_USARTx_CLK_ENABLE();
    STDIO_UART_USARTx_FORCE_RESET();
    STDIO_UART_USARTx_RELEASE_RESET();

    UART_HandleTypeDef h_uart = {
        .Instance = STDIO_UART_USARTx,
        .Init.BaudRate = 115200ul,
        .Init.WordLength = UART_WORDLENGTH_8B,
        .Init.StopBits = UART_STOPBITS_1,
        .Init.Parity = UART_PARITY_NONE,
        .Init.Mode = UART_MODE_TX_RX,
        .Init.HwFlowCtl = UART_HWCONTROL_NONE,
        .Init.OverSampling = UART_OVERSAMPLING_16,
    };

    HAL_UART_DeInit(&h_uart);

    stdio_uart_tx_pin_init();
    stdio_uart_rx_pin_init();

    HAL_UART_Init(&h_uart);

    uint16_t len = (uint16_t)snprintf(_uart_tx_buffer, sizeof(_uart_tx_buffer), "\r\n  Assertion failed in file: ""%s on line: %u\r\n", file, line);
    HAL_UART_Transmit(&h_uart, (uint8_t *)_uart_tx_buffer, len, 0xFFFFFFFFul);

#ifdef DEBUG_ASSERT_BREAKPOINT
    DEBUG_BREAKPOINT(1);
#endif
    while (1)
    {

    }
}



/** @} */
