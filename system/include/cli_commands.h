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

void cli_init_command_bindings(void);
void cli_command_clear_terminal(EmbeddedCli *cli, char *args, void *context);

#ifdef __cplusplus
}
#endif
#endif /* __CLI_COMMANDS_H__ */

