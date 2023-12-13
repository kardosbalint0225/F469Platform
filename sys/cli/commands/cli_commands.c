/*
 * cli_commands.c
 *
 *  Created on: 2023. jul. 15.
 *      Author: Balint
 */
#include "cli.h"
#include "cli_commands.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

extern void cli_command_runtime_stats(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_task_stats(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_sysinfo(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_ls(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_get_date(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_get_time(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_set_date(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_set_time(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_version(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_heapstats(EmbeddedCli *cli, char *args, void *context);

static CliCommandBinding clear_binding = {
    .name = "clear",
    .help = "Clears the console",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_clear_terminal
};

static CliCommandBinding runtime_stats_binding = {
    .name = "runtimestats",
    .help = "Displays how much processing time each FreeRTOS task has used",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_runtime_stats
};

static CliCommandBinding task_stats_binding = {
    .name = "taskstats",
    .help = "Displays a table showing the state of each FreeRTOS task",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_task_stats
};

static CliCommandBinding get_date_binding = {
    .name = "date",
    .help = "Displays the date in yyyy.mm.dd. format",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_get_date
};

static CliCommandBinding get_time_binding = {
    .name = "time",
    .help = "Displays the time in hh:mm:ss format. Optional -ms argument can be used to display the millisecond component",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_get_time
};

static CliCommandBinding set_date_binding = {
    .name = "setdate",
    .help = "setdate <yyyy.mm.dd.>: Sets the current date",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_set_date
};

static CliCommandBinding set_time_binding = {
    .name = "settime",
    .help = "settime <hh:mm:ss>: Sets the current time",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_set_time
};

static CliCommandBinding sysinfo_binding = {
    .name = "sysinfo",
    .help = "Displays system related informations",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_sysinfo
};

static CliCommandBinding version_binding = {
    .name = "version",
    .help = "Displays the application version and the used library versions",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_version
};

static CliCommandBinding heapstats_binding = {
    .name = "heapstats",
    .help = "Displays the state of the heap",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_heapstats
};

static CliCommandBinding ls_binding = {
    .name = "ls",
    .help = "Lists all files and folders in the current working directory",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_ls
};

/**
 * @brief  Adds the command bindings to the cli instance
 * @param  None
 * @retval None
 * @note   -
 */
void cli_init_command_bindings(void)
{
    EmbeddedCli *cli = cli_get_pointer();
    embeddedCliAddBinding(cli, clear_binding);
    embeddedCliAddBinding(cli, runtime_stats_binding);
    embeddedCliAddBinding(cli, task_stats_binding);
    embeddedCliAddBinding(cli, get_date_binding);
    embeddedCliAddBinding(cli, get_time_binding);
    embeddedCliAddBinding(cli, set_date_binding);
    embeddedCliAddBinding(cli, set_time_binding);
    embeddedCliAddBinding(cli, sysinfo_binding);
    embeddedCliAddBinding(cli, version_binding);
    embeddedCliAddBinding(cli, heapstats_binding);
    embeddedCliAddBinding(cli, ls_binding);
}

/**
 * @brief  Clears the terminal
 * @param  cli (not used)
 * @param  args (not used)
 * @param  context (not used)
 * @retval None
 * @note   -
 */
void cli_command_clear_terminal(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)args;
    (void)context;

    const char * const clear_string = "\33[2J";
    printf("%s", clear_string);
}








