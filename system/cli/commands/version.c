/**
 * MIT License
 *
 * Copyright (c) 2024 Balint Kardos
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/**
 * @ingroup     system_cli
 *
 * @file        version.c
 * @brief       Version Information Command
 */
#include "embedded_cli.h"
#include "library_versions.h"

#include <stdio.h>

/**
 * @brief Prints version information for various software components.
 *
 * This function prints the version information for the
 *     FreeRTOS Kernel,
 *     STM324 HAL Driver,
 *     Embedded CLI library,
 *     CMSIS GCC,
 *     Newlib nano,
 *     FatFs
 *     TODO:
 * to the console.
 *
 * @param cli     Pointer to the EmbeddedCli instance (unused).
 * @param args    Pointer to the command arguments (unused).
 * @param context Pointer to the context (unused).
 */
void cli_command_version(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)args;
    (void)context;

    printf("    FreeRTOS Kernel Version : %s\r\n", FREERTOS_KERNEL_VERSION);
    printf("    HAL Driver Version      : %s\r\n", STM32F4XX_HAL_DRIVER_VERSION);
    printf("    Embedded CLI Version    : %s\r\n", EMBEDDED_CLI_VERSION);
    printf("    CMSIS GCC Version       : %s\r\n", CMSIS_GCC_VERSION);
}
