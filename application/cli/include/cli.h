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

enum _CLI_ERROR
{
    CLI_ERROR_IO_READ_TASK_CREATE = 0x00000001UL,
    CLI_ERROR_PROCESS_TASK_CREATE = 0x00000002UL,
    CLI_ERROR_CLI_POINTER_IS_NULL = 0x00000004UL,
    CLI_ERROR_RX_QUEUE_RECEIVE    = 0x00000008UL,

    CLI_ERROR_UINT_MAX            = 0xFFFFFFFFUL,
};

void         cli_init(void);
uint32_t     cli_get_error(void);
EmbeddedCli *cli_get_pointer(void);

#endif /* _CLI_H_ */

