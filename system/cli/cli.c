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
 * @file        cli.c
 * @brief       Command Line Interface (CLI) Module
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

#include <stdio.h>
#include "stdio_base.h"
#include "stm32f4xx_hal.h"

static EmbeddedCli *_cli;
static CLI_UINT _cli_buffer[BYTES_TO_CLI_UINTS(CLI_BUFFER_SIZE)];

static StackType_t _cli_io_read_task_stack[CLI_IO_READ_TASK_STACK_SIZE];
static StaticTask_t _cli_io_read_task_tcb;
static TaskHandle_t h_cli_io_read_task = NULL;

static StackType_t _cli_process_task_stack[CLI_PROCESS_TASK_STACK_SIZE];
static StaticTask_t _cli_process_task_tcb;
static TaskHandle_t h_cli_process_task = NULL;

static StaticQueue_t _cli_rx_queue_struct;
static uint8_t _cli_rx_queue_storage[CLI_RX_QUEUE_LENGTH * sizeof(uint8_t *)];
static QueueHandle_t cli_rx_queue = NULL;

static void cli_io_read_task(void *params);
static void cli_process_task(void *params);
static void cli_write_char(EmbeddedCli *cli, char c);

void cli_init(void)
{
    _cli = NULL;

    cli_rx_queue = xQueueCreateStatic(CLI_RX_QUEUE_LENGTH,
                                      sizeof(uint8_t),
                                      _cli_rx_queue_storage,
                                      &_cli_rx_queue_struct);
    assert(cli_rx_queue);

    int ret = stdio_add_stdin_listener(cli_rx_queue);
    assert(0 == ret);

    h_cli_io_read_task = xTaskCreateStatic(cli_io_read_task,
                                           "CLI IO Read",
                                           CLI_IO_READ_TASK_STACK_SIZE,
                                           NULL,
                                           CLI_IO_READ_TASK_PRIORITY,
                                           _cli_io_read_task_stack,
                                           &_cli_io_read_task_tcb);
    assert(h_cli_io_read_task);

    h_cli_process_task = xTaskCreateStatic(cli_process_task,
                                           "CLI Process",
                                           CLI_PROCESS_TASK_STACK_SIZE,
                                           NULL,
                                           CLI_PROCESS_TASK_PRIORITY,
                                           _cli_process_task_stack,
                                           &_cli_process_task_tcb);
    assert(h_cli_process_task);

    EmbeddedCliConfig *config = embeddedCliDefaultConfig();
    config->cliBuffer = _cli_buffer;
    config->cliBufferSize = CLI_BUFFER_SIZE;
    config->rxBufferSize = CLI_RX_BUFFER_SIZE;
    config->cmdBufferSize = CLI_CMD_BUFFER_SIZE;
    config->historyBufferSize = CLI_HISTORY_SIZE;
    config->maxBindingCount = CLI_MAX_BINDING_COUNT;

    _cli = embeddedCliNew(config);
    assert(_cli);
    _cli->writeChar = cli_write_char;

    cli_init_command_bindings(_cli);
    cli_command_clear_terminal(_cli, NULL, NULL);
}

void cli_deinit(void)
{
    vTaskDelete(h_cli_io_read_task);
    vTaskDelete(h_cli_process_task);
    vQueueDelete(cli_rx_queue);
    h_cli_io_read_task = NULL;
    h_cli_process_task = NULL;
    cli_rx_queue = NULL;
}

/**
 * @brief  Task function responsible for receiving characters and passing them
 *         to the Embedded CLI library
 *
 * @param  params Task parameters (not used).
 */
static void cli_io_read_task(void *params)
{
    (void)params;

    for ( ;; )
    {
        char c;
        BaseType_t ret = xQueueReceive(cli_rx_queue, &c, portMAX_DELAY);
        assert(ret);

        embeddedCliReceiveChar(_cli, c);
    }
}

/**
 * @brief  Task function responsible for processing the received characters
 *
 * @param  params Task parameters (not used).
 */
static void cli_process_task(void *params)
{
    (void)params;

    for ( ;; )
    {
        embeddedCliProcess(_cli);
        vTaskDelay(10);
    }
}

/**
 * @brief  Writes single character to the console
 *
 * @param  cli Pointer to the CLI instance
 * @param  c   Character to write to the console
 */
static void cli_write_char(EmbeddedCli *cli, char c)
{
    (void)cli;
    stdio_write((char *)&c, 1);
}


