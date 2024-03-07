/*
 * Copyright (C) 2015 Kaspar Schleiser <kaspar@schleiser.de>
 *               2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    sys_stdio STDIO abstraction
 * @ingroup     sys
 *
 * @brief       Simple standard input/output (STDIO) abstraction for RIOT
 *
 * @{
 * @file
 *
 * @author      Kaspar Schleiser <kaspar@schleiser.de>
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

#ifndef STDIO_BASE_H
#define STDIO_BASE_H

#include <unistd.h>

#include "modules.h"

#include "FreeRTOS.h"
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief initialize the module
 */
void stdio_init(void);

/**
 * @brief De-initialize the module
 */
void stdio_deinit(void);

/**
 * @brief Adds the given queue handle to the listeners list
 *
 * @param[in] hqueue QueueHandle_t handle to be added to the listeners list
 *
 * @return 0 on success
 * @return < 0 on error
 *
 * @note The CLI adds its input queue handle to the listeners list to
 *       receive the characters when the stdin is not used by other tasks
 *       If a task uses a function which relies on stdin then the received bytes
 *       are passed to the stdin queue bypassing the CLI input queue
 */
int stdio_add_stdin_listener(const QueueHandle_t hqueue);

#if IS_USED(MODULE_STDIO_AVAILABLE) || DOXYGEN
/**
 * @brief   Get the number of bytes available for reading from stdio.
 *
 * @warning This function is only available if the implementation supports
 *          it and the @c stdio_available module is enabled.
 *
 * @return  number of available bytes
 */
int stdio_available(void);
#endif

/**
 * @brief read @p len bytes from stdio into @p buffer
 *
 * @param[out]  buffer  buffer to read into
 * @param[in]   max_len nr of bytes to read
 *
 * @return nr of bytes read
 * @return <0 on error
 */
ssize_t stdio_read(void* buffer, size_t max_len);

/**
 * @brief write @p len bytes from @p buffer to stdio
 *
 * @param[in]   buffer  buffer to read from
 * @param[in]   len     nr of bytes to write
 *
 * @return nr of bytes written
 * @return <0 on error
 */
ssize_t stdio_write(const void* buffer, size_t len);

/**
 * @brief write @p len bytes from @p buffer to stdio in blocking mode
 *
 * @param[in]   buffer  buffer to read from
 * @param[in]   len     nr of bytes to write
 *
 * @return nr of bytes written
 * @return <0 on error
 */
ssize_t stdio_write_blocking(const void* buffer, size_t len);

#ifdef __cplusplus
}
#endif
/** @} */
#endif /* STDIO_BASE_H */
