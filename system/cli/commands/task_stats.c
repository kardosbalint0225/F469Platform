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
 * @file        task_stats.c
 * @brief       Task Statistics Commands
 */
#include "embedded_cli.h"
#include "cli_config.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>

static const char * const _task_states[] = {
    "Running", "Ready", "Blocked", "Suspended", "Deleted", "Invalid"
};

extern uint32_t uxTaskGetStackSize(TaskHandle_t xTask);

/**
 * @brief Function that is executed when the runtimestats command is entered.
 *        Displays the absolute and relative times used by each task.
 *
 * This function retrieves runtime statistics for all FreeRTOS tasks including
 * task ID, name, state, priority, absolute time used, and CPU time percentage.
 * It then prints these statistics to the console.
 *
 * @param cli     Pointer to the EmbeddedCli instance (unused).
 * @param args    Pointer to the command arguments (unused).
 * @param context Pointer to the context (unused).
 */
void cli_command_runtime_stats(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)args;
    (void)context;

    const uint32_t number_of_tasks = uxTaskGetNumberOfTasks();
    TaskStatus_t *tasks = pvPortMalloc(number_of_tasks * sizeof(TaskStatus_t));
    uint32_t total_runtime;
    char cpu_usage_str[8];

    if (NULL == tasks)
    {
        printf("  Not enough memory.\r\n");
        return;
    }

    printf("  ID |      Name       |   State   | Priority |  Abs time  |   Time %% \r\n");
    printf("  ---+-----------------+-----------+----------+------------+------------\r\n");

    uxTaskGetSystemState(tasks, number_of_tasks, &total_runtime);
    /* For percentage calculations. */
    total_runtime /= 100ul;
    /* Avoid divide by zero errors. */
    if (0ul == total_runtime)
    {
        total_runtime = 1ul;
    }

    for (uint32_t i = 0; i < number_of_tasks; i++)
    {
        const uint32_t cpu_usage = tasks[i].ulRunTimeCounter / total_runtime;
        if (cpu_usage > 0ul)
        {
            snprintf(cpu_usage_str, sizeof(cpu_usage_str), "%3lu%%", cpu_usage);
        }
        else
        {
            snprintf(cpu_usage_str, sizeof(cpu_usage_str), "<1%%");
        }

        printf("  %2lu | %-15s | %-9s | %8lu | %10lu | %8s\r\n",
               tasks[i].xTaskNumber,
               tasks[i].pcTaskName,
               _task_states[tasks[i].eCurrentState],
               tasks[i].uxCurrentPriority,
               tasks[i].ulRunTimeCounter,
               cpu_usage_str);
    }

    vPortFree(tasks);
}

/**
 * @brief Function that is executed when the taskstats command is entered.
 *        Displays the state, priority and stack usage of each task.
 *
 * This function retrieves statistics for all FreeRTOS tasks, including
 * task ID, name, state, priority, stack size, and stack usage percentage.
 * It then prints these statistics to the console.
 *
 * @param cli     Pointer to the EmbeddedCli instance (unused).
 * @param args    Pointer to the command arguments (unused).
 * @param context Pointer to the context (unused).
 */
void cli_command_task_stats(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)args;
    (void)context;

    const uint32_t number_of_tasks = uxTaskGetNumberOfTasks();
    TaskStatus_t *tasks = pvPortMalloc(number_of_tasks * sizeof(TaskStatus_t));

    if (NULL == tasks)
    {
        printf("  Not enough memory.\r\n");
        return;
    }

    printf("  ID |      Name       |   State   | Priority | Stack size |   Used %% \r\n");
    printf("  ---+-----------------+-----------+----------+------------+------------\r\n");

    uxTaskGetSystemState(tasks, number_of_tasks, NULL);

    for (uint32_t i = 0; i < number_of_tasks; i++)
    {
        const uint32_t stack_size = uxTaskGetStackSize(tasks[i].xHandle);
        const uint32_t free = tasks[i].usStackHighWaterMark * sizeof(StackType_t);
        const float stack_usage = 100.0f * ((float)(stack_size - free) / (float)stack_size);
        printf("  %2lu | %-15s | %-9s | %8lu | %8lu B | %6.2f %%\r\n",
               tasks[i].xTaskNumber,
               tasks[i].pcTaskName,
               _task_states[tasks[i].eCurrentState],
               tasks[i].uxCurrentPriority,
               stack_size, stack_usage);
    }

    vPortFree(tasks);
}
