/*
 * Original work Copyright (C) 2015 INRIA
 *                             2015 Eistec AB
 *                             2015 Kaspar Schleiser <kaspar@schleiser.de>
 * Modified work Copyright (C) 2024 Balint Kardos
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the root
 * directory of this software component or visit
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html for more details
 *
 * The original work can be found in the RIOT-OS 2024.01 branch:
 *     RIOT-OS repository: https://github.com/RIOT-OS/RIOT/tree/2024.01-branch
 *     panic.c:            https://github.com/RIOT-OS/RIOT/blob/2024.01-branch/core/lib/panic.c
 *
 * The original authors of panic.c are:
 *     Kévin Roussel <Kevin.Roussel@inria.fr>
 *     Oliver Hahm <oliver.hahm@inria.fr>
 *     Joakim Nohlgård <joakim.nohlgard@eistec.se>
 *     Kaspar Schleiser <kaspar@schleiser.de>
 */

/**
 * @ingroup     core_util
 * @{
 *
 * @file        panic.c
 * @brief       Crash handling functions
 *
 * @}
 */

#include "FreeRTOS.h"
#include "task.h"
#include "panic.h"
#include "debug.h"
#include "core_config.h"
#include "stm32f4xx_hal.h"

#include <stdarg.h>
#include <stdio.h>

static void _panic_init(UART_HandleTypeDef *huart);
static void _uart_periph_init(UART_HandleTypeDef *huart);
static void _uart_msp_init(UART_HandleTypeDef *huart);

/* flag preventing "recursive crash printing loop" */
static int _crashed = 0;
static char _tx_buffer[513];

__NORETURN void panic(const char *message, ...)
{
    if (0 == _crashed)
    {
        /* print panic message to console (if possible) */
        _crashed = 1;

        UART_HandleTypeDef h_panic_uart = { 0 };
        _panic_init(&h_panic_uart);

        va_list va;
        va_start(va, message);
        size_t len = vsnprintf(_tx_buffer, sizeof(_tx_buffer), message, va);
        va_end(va);

        HAL_StatusTypeDef ret;
        ret = HAL_UART_Transmit(&h_panic_uart, (uint8_t *)_tx_buffer, (uint16_t)len, 0xFFFFFFFFul);
        (void)ret;
    }

#ifdef DEBUG_ASSERT_BREAKPOINT
    DEBUG_BREAKPOINT(1);
#endif

    while (1)
    {

    }
}

static void _panic_init(UART_HandleTypeDef *huart)
{
    vTaskSuspendAll();
    __disable_irq();

    __HAL_RCC_DMA1_CLK_DISABLE();
    __HAL_RCC_DMA2_CLK_DISABLE();

    _uart_periph_init(huart);
}

static void _uart_periph_init(UART_HandleTypeDef *huart)
{
    huart->Instance = PANIC_USARTx;
    huart->Init.BaudRate = 115200ul;
    huart->Init.WordLength = UART_WORDLENGTH_8B;
    huart->Init.StopBits = UART_STOPBITS_1;
    huart->Init.Parity = UART_PARITY_NONE;
    huart->Init.Mode = UART_MODE_TX_RX;
    huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart->Init.OverSampling = UART_OVERSAMPLING_16;

    HAL_StatusTypeDef ret;
    ret = HAL_UART_RegisterCallback(huart, HAL_UART_MSPINIT_CB_ID, _uart_msp_init);
    (void)ret;
    ret = HAL_UART_Init(huart);
    (void)ret;
}

static void _uart_msp_init(UART_HandleTypeDef *huart)
{
    PANIC_UART_CLK_ENABLE();
    PANIC_UART_FORCE_RESET();
    PANIC_UART_RELEASE_RESET();

    GPIO_InitTypeDef panic_uart_tx_pin = {
        .Pin = PANIC_UART_TX_PIN,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
        .Alternate = PANIC_UART_GPIO_AFx_USARTx,
    };
    PANIC_UART_TX_GPIO_CLK_ENABLE();
    HAL_GPIO_Init(PANIC_UART_TX_GPIO_PORT, &panic_uart_tx_pin);
}


