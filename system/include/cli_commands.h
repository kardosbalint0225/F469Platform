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
 * @file        cli_commands.h
 * @brief       CLI Command Bindings
 */

#ifndef __CLI_COMMANDS_H__
#define __CLI_COMMANDS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "embedded_cli.h"

/**
 * @brief Initializes command bindings for the Command Line Interface (CLI).
 *
 * This function initializes command bindings for the given Command Line Interface (CLI)
 * instance.
 *
 * @param cli Pointer to the EmbeddedCli instance to which command bindings will be added.
 */
void cli_init_command_bindings(EmbeddedCli *cli);

extern void cli_command_runtime_stats(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_task_stats(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_sysinfo(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_get_date(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_get_time(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_set_date(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_set_time(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_version(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_memstat(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_ls(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_cd(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_cp(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_r(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_rm(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_mkdir(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_clear_terminal(EmbeddedCli *cli, char *args, void *context);

#ifdef __cplusplus
}
#endif
#endif /* __CLI_COMMANDS_H__ */

