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
 * @file        cli_commands.c
 * @brief       CLI Command Bindings
 */
#include "cli.h"
#include "cli_commands.h"
#include <assert.h>
#include <stddef.h>
#include <stdbool.h>

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

static CliCommandBinding memstat_binding = {
    .name = "memstat",
    .help = "Displays the detailed state of the application and system memory",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_memstat
};

static CliCommandBinding ls_binding = {
    .name = "ls",
    .help = "Lists all files and folders in the current working directory",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_ls
};

static CliCommandBinding cd_binding = {
    .name = "cd",
    .help = "todo",//TODO
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_cd
};

static CliCommandBinding cp_binding = {
    .name = "cp",
    .help = "todo",//TODO
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_cp
};

static CliCommandBinding r_binding = {
    .name = "r",
    .help = "todo",//TODO
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_r
};

static CliCommandBinding rm_binding = {
    .name = "rm",
    .help = "todo",//TODO
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_rm
};

static CliCommandBinding mv_binding = {
    .name = "mv",
    .help = "todo",//TODO
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_mv
};

static CliCommandBinding mkdir_binding = {
    .name = "mkdir",
    .help = "todo",//TODO
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_mkdir
};

void cli_init_command_bindings(EmbeddedCli *cli)
{
    assert(cli);
    bool ret;

    ret = embeddedCliAddBinding(cli, clear_binding);
    assert(ret);
    ret = embeddedCliAddBinding(cli, runtime_stats_binding);
    assert(ret);
    ret = embeddedCliAddBinding(cli, task_stats_binding);
    assert(ret);
    ret = embeddedCliAddBinding(cli, get_date_binding);
    assert(ret);
    ret = embeddedCliAddBinding(cli, get_time_binding);
    assert(ret);
    ret = embeddedCliAddBinding(cli, set_date_binding);
    assert(ret);
    ret = embeddedCliAddBinding(cli, set_time_binding);
    assert(ret);
    ret = embeddedCliAddBinding(cli, sysinfo_binding);
    assert(ret);
    ret = embeddedCliAddBinding(cli, version_binding);
    assert(ret);
    ret = embeddedCliAddBinding(cli, memstat_binding);
    assert(ret);
    ret = embeddedCliAddBinding(cli, ls_binding);
    assert(ret);
    ret = embeddedCliAddBinding(cli, cd_binding);
    assert(ret);
    ret = embeddedCliAddBinding(cli, cp_binding);
    assert(ret);
    ret = embeddedCliAddBinding(cli, r_binding);
    assert(ret);
    ret = embeddedCliAddBinding(cli, rm_binding);
    assert(ret);
    ret = embeddedCliAddBinding(cli, mv_binding);
    assert(ret);
    ret = embeddedCliAddBinding(cli, mkdir_binding);
    assert(ret);
}










