/*
 * cli.h
 *
 *  Created on: 2023. jul. 15.
 *      Author: Balint
 */

#ifndef __CLI_H__
#define __CLI_H__

#include <stdint.h>
#include "embedded_cli.h"

void         cli_init(void);
EmbeddedCli *cli_get_pointer(void);

#endif /* __CLI_H__ */

