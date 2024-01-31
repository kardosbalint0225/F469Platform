/*
 * cli.h
 *
 *  Created on: 2023. jul. 15.
 *      Author: Balint
 */

#ifndef _CLI_H_
#define _CLI_H_

#include <stdint.h>
#include "embedded_cli.h"

void         cli_init(void);
EmbeddedCli *cli_get_pointer(void);

#endif /* _CLI_H_ */

