/*
 * cli_config.h
 *
 *  Created on: 2023. jul. 15.
 *      Author: Balint
 */

#ifndef __CLI_CONFIG_H__
#define __CLI_CONFIG_H__

// Definitions for CLI sizes
#define CLI_BUFFER_SIZE                2048
#define CLI_RX_BUFFER_SIZE             64
#define CLI_CMD_BUFFER_SIZE            64
#define CLI_HISTORY_SIZE               32
#define CLI_MAX_BINDING_COUNT          32

/**
 * Definition of the cli_printf() buffer size.
 * Can make smaller to decrease RAM usage,
 * make larger to be able to print longer strings.
 */
#define CLI_PRINT_BUFFER_SIZE          512

#define CLI_IO_READ_TASK_PRIORITY      3ul
#define CLI_IO_READ_TASK_STACK_SIZE    (configMINIMAL_STACK_SIZE)
#define CLI_PROCESS_TASK_PRIORITY      3ul
#define CLI_PROCESS_TASK_STACK_SIZE    (configMINIMAL_STACK_SIZE + 31 * configMINIMAL_STACK_SIZE)
#define CLI_RX_QUEUE_LENGTH            1ul

#endif /* __CLI_CONFIG_H__ */

