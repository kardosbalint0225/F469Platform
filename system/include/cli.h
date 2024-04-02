/*
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
 * @{
 * @file        cli.h
 * @brief       Command Line Interface (CLI) Module
 */

#ifndef __CLI_H__
#define __CLI_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the CLI module.
 *
 * This function sets up the CLI by creating the required tasks, queues, and buffers,
 * and configuring the EmbeddedCLI with the settings defined in cli_config.h.
 * It also adds the command bindings and clears the terminal screen.
 */
void cli_init(void);

/**
 * @brief De-initializes the CLI module.
 *
 * This function deallocates resources used by the CLI, including tasks and queues.
 * It also performs cleanup operations such as deleting tasks and queues and setting
 * pointers to NULL.
 */
void cli_deinit(void);

#ifdef __cplusplus
}
#endif
#endif /* __CLI_H__ */
/** @} */

