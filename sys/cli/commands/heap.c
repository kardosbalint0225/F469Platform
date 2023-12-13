/*
 * heap.c
 *
 *  Created on: 2023. dec. 12.
 *      Author: Balint
 */
#include "embedded_cli.h"

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
