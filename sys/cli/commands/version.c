/*
 * version.c
 *
 *  Created on: 2023. dec. 12.
 *      Author: Balint
 */
#include "embedded_cli.h"
#include "library_versions.h"

#include <stdio.h>

/**
 * @brief  Function that is executed when the version command is entered.
 *         Displays the application version and the used library versions
 * @param  cli (not used)
 * @param  args (not used)
 * @param  context (not used)
 * @retval None
 * @note   -
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
