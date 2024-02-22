/*
 * commands.h
 *
 *  Created on: 2023. jul. 15.
 *      Author: Balint
 */

#ifndef __CLI_COMMANDS_H__
#define __CLI_COMMANDS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "embedded_cli.h"

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
extern void cli_command_mv(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_mkdir(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_clear_terminal(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_assert(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_stdintest(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_malloctest(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_hardfaulttest(EmbeddedCli *cli, char *args, void *context);

#ifdef __cplusplus
}
#endif
#endif /* __CLI_COMMANDS_H__ */

