/*
 * task.c
 *
 *  Created on: 2023. dec. 12.
 *      Author: Balint
 */
#include "embedded_cli.h"
#include "cli_config.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>

static char _buffer[CLI_PRINT_BUFFER_SIZE];

/**
 * @brief  Function that is executed when runtimestats command entered
 *         Displays the absolute and relative times of each task
 *
 * @param  cli (not used)
 * @param  args (not used)
 * @param  context (not used)
 *
 * @retval None
 */
void cli_command_runtime_stats(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)args;
    (void)context;

    printf("Task            Abs Time        %% Time\r\n");
    printf("****************************************\r\n");

    vTaskGetRunTimeStats(_buffer);

    printf("%s\r\n", _buffer);
}

/**
 * @brief  Function that is executed when taskstats command entered
 *         Displays the state, priority and stack usage of each task
 *
 * @param  cli (not used)
 * @param  args (not used)
 * @param  context (not used)
 *
 * @retval None
 */
void cli_command_task_stats(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)args;
    (void)context;

    printf("Task          State  Priority   Stack   #\r\n");
    printf("************************************************\r\n");

    vTaskList(_buffer);

    printf("%s\r\n", _buffer);
}
