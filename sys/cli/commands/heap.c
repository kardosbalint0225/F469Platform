/*
 * heap.c
 *
 *  Created on: 2023. dec. 12.
 *      Author: Balint
 */
#include "embedded_cli.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>

static TaskStatus_t *tasks = NULL;
extern uint32_t uxTaskGetStackSize(TaskHandle_t xTask);
static void print_system_heap_statistics(void);
static void print_tasks_stack_usage_statistics(void);

extern void heap_stats(void);

/**
 * @brief  Function that is executed when the heapstats command is entered.
 *         Displays the state of the heap
 * @param  cli (not used)
 * @param  args (not used)
 * @param  context (not used)
 * @retval None
 * @note   -
 */
void cli_command_heapstats(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)args;
    (void)context;

    heap_stats();
}

/**
 * @brief  Function that is executed when the memstat command is entered.
 *         Displays the detailed state of the application memory
 * @param  cli (not used)
 * @param  args (not used)
 * @param  context (not used)
 * @retval None
 * @note   -
 */
void cli_command_memstat(EmbeddedCli *cli, char *args, void *context)
{
    print_system_heap_statistics();
    print_tasks_stack_usage_statistics();


    // Application Heap statistics:

    // Application Stack statistics:
}

static void print_system_heap_statistics(void)
{
    // System (FreRTOS) Heap statistics:
    const size_t size = configTOTAL_HEAP_SIZE;
    HeapStats_t stats;
    vPortGetHeapStats(&stats);
    const float heap_usage = 100.0f * ((float)stats.xAvailableHeapSpaceInBytes / (float)size);
    printf("\r\n  System (FreeRTOS) Heap statistics:\r\n");
    printf("    size = %u [byte]\r\n", size);
    printf("    free = %u [byte] | %2.2f%%\r\n", stats.xAvailableHeapSpaceInBytes, heap_usage);
}

static void print_tasks_stack_usage_statistics(void)
{
    // FreeRTOS tasks stack usage statistics:
    const uint32_t number_of_tasks = uxTaskGetNumberOfTasks();
    tasks = pvPortMalloc(number_of_tasks * sizeof(TaskStatus_t));

    if (NULL == tasks)
    {
        printf("    Not enough memory.\r\n");
        return;
    }

    uxTaskGetSystemState(tasks, number_of_tasks, NULL);

    for (uint32_t i = 0; i < number_of_tasks; i++)
    {
        const uint32_t stack_size = uxTaskGetStackSize(tasks[i].xHandle);
        const uint32_t free =
        //const float stack_usage = (float)tasks[i].
    }

    vPortFree(tasks);
}








