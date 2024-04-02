/*
 * Original work Copyright (C) 2015 Kaspar Schleiser <kaspar@schleiser.de>
 *                             2018 Freie Universität Berlin
 * Modified work Copyright (C) 2024 Balint Kardos
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the root
 * directory of this software component or visit
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html for more details
 *
 * The original work can be found in the RIOT-OS 2024.01 branch:
 *     RIOT-OS repository: https://github.com/RIOT-OS/RIOT/tree/2024.01-branch
 *     stdio_base.h:       https://github.com/RIOT-OS/RIOT/blob/2024.01-branch/sys/include/stdio_base.h
 *
 * The original authors of stdio_base.h are:
 *     Kaspar Schleiser <kaspar@schleiser.de>
 *     Hauke Petersen <hauke.petersen@fu-berlin.de>
 */

/**
 * @ingroup     system_stdio
 *
 * @brief       Simple standard input/output (STDIO) abstraction
 *
 * @{
 * @file        stdio_base.h
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
 * @brief Initialize the module
 */
void stdio_init(void);

/**
 * @brief De-initialize the module
 */
void stdio_deinit(void);

/**
 * @brief Adds the given queue handle to the listeners list
 *
 * @param[in] hqueue handle to be added to the listeners list
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
 * @brief read @p len bytes from stdio uart into @p buffer
 *
 * @param[out]  buffer  buffer to read into
 * @param[in]   max_len nr of bytes to read
 *
 * @return nr of bytes read
 * @return <0 on error
 */
ssize_t stdio_read(void* buffer, size_t max_len);

/**
 * @brief write @p len bytes from @p buffer into uart
 *
 * @param[in]   buffer  buffer to read from
 * @param[in]   len     nr of bytes to write
 *
 * @return nr of bytes written
 * @return <0 on error
 */
ssize_t stdio_write(const void* buffer, size_t len);

#ifdef __cplusplus
}
#endif
#endif /* STDIO_BASE_H */
/** @} */
