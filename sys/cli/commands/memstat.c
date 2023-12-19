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
#include <malloc.h>

#define SRAM3_END    0x2004FFFFul

static TaskStatus_t *tasks = NULL;
extern uint32_t uxTaskGetStackSize(TaskHandle_t xTask);
static void print_system_heap_statistics(void);
static void print_tasks_stack_usage_statistics(void);
static void print_application_heap_statistics(void);

/**
 * @brief  Function that is executed when the memstat command is entered.
 *         Displays the detailed state of the application and system memory
 * @param  cli (not used)
 * @param  args (not used)
 * @param  context (not used)
 * @retval None
 * @note   -
 */
void cli_command_memstat(EmbeddedCli *cli, char *args, void *context)
{
    printf("\r\n  Internal memory layout:\r\n\r\n");
    printf("           RAM         |      Base      |      End       |      Size    \r\n");
    printf("  ---------------------+----------------+----------------+--------------\r\n");

    printf("   CCM data RAM        |   0x%08lx   |   0x%08lx   | %9lu KB\r\n",
           CCMDATARAM_BASE, CCMDATARAM_END, (CCMDATARAM_END + 1 - CCMDATARAM_BASE) / 1024);
    printf("   SRAM1               |   0x%08lx   |   0x%08lx   | %9lu KB\r\n",
           SRAM1_BASE, SRAM2_BASE - 1, (SRAM2_BASE - SRAM1_BASE) / 1024);
    printf("   SRAM2               |   0x%08lx   |   0x%08lx   | %9lu KB\r\n",
           SRAM2_BASE, SRAM3_BASE - 1, (SRAM3_BASE - SRAM2_BASE) / 1024);
    printf("   SRAM3               |   0x%08lx   |   0x%08lx   | %9lu KB\r\n",
           SRAM3_BASE, SRAM3_END, (SRAM3_END + 1 - SRAM3_BASE) / 1024);

    print_system_heap_statistics();
    print_tasks_stack_usage_statistics();
    print_application_heap_statistics();



    // Application Stack statistics:
}

static void print_system_heap_statistics(void)
{
    // System (FreRTOS) Heap statistics:
    const size_t size = configTOTAL_HEAP_SIZE;
    HeapStats_t stats;
    vPortGetHeapStats(&stats);

    printf("\r\n\r\n  System (FreeRTOS) Heap statistics:\r\n\r\n");
    printf("       Size     |     Used     |     Free     |    Used  %% |    Free  %% \r\n");
    printf("  --------------+--------------+--------------+------------+------------\r\n");
    printf("   %10u B | %10u B | %10u B | %8.4f %% | %8.4f %%\r\n",
           size,
           size - stats.xAvailableHeapSpaceInBytes,
           stats.xAvailableHeapSpaceInBytes,
           100.0f * ((float)(size - stats.xAvailableHeapSpaceInBytes) / (float)size),
           100.0f * ((float)stats.xAvailableHeapSpaceInBytes / (float)size));
}

static void print_tasks_stack_usage_statistics(void)
{
    // FreeRTOS tasks stack usage statistics:
    const uint32_t number_of_tasks = uxTaskGetNumberOfTasks();
    tasks = pvPortMalloc(number_of_tasks * sizeof(TaskStatus_t));

    if (NULL == tasks)
    {
        printf("  Not enough memory.\r\n");
        return;
    }

    printf("\r\n\r\n  FreeRTOS tasks stack usage statistics:\r\n\r\n");
    printf("  ID |      Name       | Stack size |    Free    |   Used %% |  Address\r\n");
    printf("  ---+-----------------+------------+------------+----------+-----------\r\n");

    uxTaskGetSystemState(tasks, number_of_tasks, NULL);

    for (uint32_t i = 0; i < number_of_tasks; i++)
    {
        const uint32_t stack_size = uxTaskGetStackSize(tasks[i].xHandle);
        const uint32_t free = tasks[i].usStackHighWaterMark * sizeof(StackType_t);
        const float stack_usage = 100.0f * ((float)(stack_size - free) / (float)stack_size);
        printf("  %2lu | %-15s | %8lu B | %8lu B | %6.2f %% | 0x%08lx\r\n",
               tasks[i].xTaskNumber, tasks[i].pcTaskName, stack_size, free, stack_usage, (uint32_t)tasks[i].pxStackBase);
    }

    vPortFree(tasks);
}

static void print_application_heap_statistics(void)
{
    // Application Heap statistics:

    extern uint8_t _end; /* Symbol defined in the linker script */
    extern uint8_t _estack; /* Symbol defined in the linker script */
    extern uint32_t _Min_Stack_Size; /* Symbol defined in the linker script */

    struct mallinfo minfo = mallinfo();
    const uint32_t end = (uint32_t)&_estack - (uint32_t)&_Min_Stack_Size;
    const uint32_t start = (uint32_t)&_end;
    const uint32_t heap_size = end - start;

    printf("\r\n\r\n  Application Heap statistics: base address = 0x%08lx\r\n\r\n", start);
    printf("       Size     |     Used     |     Free     |    Used  %% |    Free  %% \r\n");
    printf("  --------------+--------------+--------------+------------+------------\r\n");
    printf("   %10lu B | %10lu B | %10lu B | %8.4f %% | %8.4f %%\r\n",
           heap_size,
           (uint32_t)minfo.uordblks,
           heap_size - (uint32_t)minfo.uordblks,
           100.0f * ((float)minfo.uordblks / (float)heap_size),
           100.0f * ((float)(heap_size - (uint32_t)minfo.uordblks) / (float)heap_size));
}






