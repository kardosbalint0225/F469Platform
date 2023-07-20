/*
 * cli.c
 *
 *  Created on: 2023. jul. 15.
 *      Author: Balint
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define EMBEDDED_CLI_IMPL
#include "embedded_cli.h"
#include "cli_commands.h"

#include "cli.h"
#include "cli_config.h"

#include "uart_console.h"
#include "stm32f4xx_hal.h"

extern QueueHandle_t h_uart_rx_queue;

// CLI buffer
static EmbeddedCli *cli;
static CLI_UINT cli_buffer[BYTES_TO_CLI_UINTS(CLI_BUFFER_SIZE)];

static uint32_t cli_error;

static StackType_t  cli_io_read_task_stack[CLI_IO_READ_TASK_STACK_SIZE];
static StaticTask_t cli_io_read_task_tcb;
static TaskHandle_t h_cli_io_read_task = NULL;

static StackType_t  cli_process_task_stack[CLI_PROCESS_TASK_STACK_SIZE];
static StaticTask_t cli_process_task_tcb;
static TaskHandle_t h_cli_process_task = NULL;

static void cli_io_read_task(void *params);
static void cli_process_task(void *params);
static void cli_write_char(EmbeddedCli *cli, char c);

void cli_init(void)
{
	cli = NULL;
	cli_error = 0UL;

    h_cli_io_read_task = xTaskCreateStatic(cli_io_read_task,
                                           "CLI IO Read",
                                           CLI_IO_READ_TASK_STACK_SIZE,
                                           NULL,
                                           CLI_IO_READ_TASK_PRIORITY,
                                           cli_io_read_task_stack,
                                           &cli_io_read_task_tcb);
    cli_error |= (NULL == h_cli_io_read_task) ? CLI_ERROR_IO_READ_TASK_CREATE : 0UL;
	assert_param(0UL == cli_error);

	h_cli_process_task = xTaskCreateStatic(cli_process_task,
	                                       "CLI Process",
	                                       CLI_PROCESS_TASK_STACK_SIZE,
	                                       NULL,
	                                       CLI_PROCESS_TASK_PRIORITY,
	                                       cli_process_task_stack,
	                                       &cli_process_task_tcb);
	cli_error |= (NULL == h_cli_process_task) ? CLI_ERROR_PROCESS_TASK_CREATE : 0UL;
	assert_param(0UL == cli_error);

    EmbeddedCliConfig *config = embeddedCliDefaultConfig();
    config->cliBuffer         = cli_buffer;
    config->cliBufferSize     = CLI_BUFFER_SIZE;
    config->rxBufferSize      = CLI_RX_BUFFER_SIZE;
    config->cmdBufferSize     = CLI_CMD_BUFFER_SIZE;
    config->historyBufferSize = CLI_HISTORY_SIZE;
    config->maxBindingCount   = CLI_MAX_BINDING_COUNT;

    cli = embeddedCliNew(config);
    cli->writeChar = cli_write_char;
    cli_error |= (NULL == cli) ? CLI_ERROR_CLI_POINTER_IS_NULL : 0UL;
    assert_param(0UL == cli_error);

    cli_init_command_bindings();
    cli_command_clear_terminal(cli, NULL, NULL);
}

void cli_deinit(void)
{
	vTaskDelete(h_cli_io_read_task);
	vTaskDelete(h_cli_process_task);
	h_cli_io_read_task = NULL;
	h_cli_process_task = NULL;
	//TODO: deinit command bindings
}

static void cli_io_read_task(void *params)
{
	(void)params;

	for( ;; )
	{
		char c;
		BaseType_t ret = xQueueReceive(h_uart_rx_queue, &c, portMAX_DELAY);
		cli_error |= (pdTRUE != ret) ? CLI_ERROR_RX_QUEUE_RECEIVE : 0UL;
		assert_param(0UL == cli_error);

		embeddedCliReceiveChar(cli, c);
	}
}

static void cli_process_task(void *params)
{
	(void)params;

	for( ;; )
	{
		embeddedCliProcess(cli);
		vTaskDelay(10);
	}
}

static void cli_write_char(EmbeddedCli *cli, char c)
{
	__uart_console_write(&c, 1);
}

EmbeddedCli *cli_get_pointer(void)
{
    return cli;
}

uint32_t cli_get_error(void)
{
	return cli_error;
}


