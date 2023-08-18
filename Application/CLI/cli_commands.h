/*
 * commands.h
 *
 *  Created on: 2023. jul. 15.
 *      Author: Balint
 */

#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include "embedded_cli.h"

void cli_init_command_bindings(void);
void cli_command_clear_terminal(EmbeddedCli *cli, char *args, void *context);

#endif /* _COMMANDS_H_ */

