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

static const char * const _task_states[] = {
    "Running", "Ready", "Blocked", "Suspended", "Deleted", "Invalid"
};

extern uint32_t uxTaskGetStackSize(TaskHandle_t xTask);

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
