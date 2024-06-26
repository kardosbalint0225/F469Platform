/*
 * Original work Copyright (C) 2014 Freie Universität Berlin
 * Modified work Copyright (C) 2024 Balint Kardos
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the root
 * directory of this software component or visit
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html for more details
 *
 * The original work can be found in the RIOT-OS 2024.01 branch:
 *     RIOT-OS repository: https://github.com/RIOT-OS/RIOT/tree/2024.01-branch
 *     debug.h:            https://github.com/RIOT-OS/RIOT/blob/2024.01-branch/core/lib/include/debug.h
 *
 * The original author of debug.h is:
 *     Kaspar Schleiser <kaspar@schleiser.de>
 */

/**
 * @ingroup     core_util
 * @{
 *
 * @file        debug.h
 * @brief       Debug-header
 *
 * @details     If *ENABLE_DEBUG* is defined inside an implementation file, all
 *              calls to ::DEBUG will work the same as *printf* and output the
 *              given information to stdout. If *ENABLE_DEBUG* is not defined,
 *              all calls to ::DEBUG will be ignored.
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>
#include "FreeRTOS.h"
#include "core_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def DEBUG_PRINT
 *
 * @brief Print debug information if the calling task stack is large enough
 *
 * Use this macro the same as `printf`. When `DEVELHELP` is defined inside an
 * implementation file, all usages of ::DEBUG_PRINT will print the given
 * information to stdout after verifying the stack is big enough. If `DEVELHELP`
 * is not set, this check is not performed. (CPU exception may occur)
 */
#ifdef DEVELHELP

#define DEBUG_PRINT(...) \
    do { \
        extern uint32_t uxTaskGetStackSize( TaskHandle_t xTask ); \
        TaskHandle_t current = xTaskGetCurrentTaskHandle(); \
        uint32_t stack_size = uxTaskGetStackSize(current); \
        if ( (current != NULL) && (stack_size >= TASK_EXTRA_STACKSIZE_PRINTF) ) \
        { \
            printf(__VA_ARGS__); \
        } \
        else \
        { \
            puts("Cannot debug, stack too small. Consider using DEBUG_PUTS()."); \
        } \
    } while (0)
#else
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#endif

/**
 * @def DEBUG_BREAKPOINT
 *
 * @brief Set a debug breakpoint
 *
 * When `DEVELHELP` is enabled, this traps the CPU and allows to debug the
 * program with e.g. `gdb`.
 * Without `DEVELHELP` this turns into a no-op.
 *
 * @warning     If no Debugger is attached, the CPU might get stuck here
 *              and consume a lot of power until reset.
 *
 * @param val   Breakpoint context for debugger, usually ignored.
 */
#ifdef DEVELHELP
#define DEBUG_BREAKPOINT(val)   __BKPT(val)
#else
#define DEBUG_BREAKPOINT(val)   (void)0
#endif

/**
 * @name Debugging defines
 * @{
 */
/**
 * @brief   This macro can be defined as 0 or other on a file-based level.
 *          @ref DEBUG() will generate output only if ENABLE_DEBUG is non-zero.
 */
#if !defined(ENABLE_DEBUG) || defined(DOXYGEN)
#define ENABLE_DEBUG 0
#endif

/**
 * @def DEBUG_FUNC
 *
 * @brief   Contains the function name if given compiler supports it.
 *          Otherwise it is an empty string.
 */
# if defined(__cplusplus) && defined(__GNUC__)
#  define DEBUG_FUNC __PRETTY_FUNCTION__
# elif __STDC_VERSION__ >= 199901L
#  define DEBUG_FUNC __func__
# elif __GNUC__ >= 2
#  define DEBUG_FUNC __FUNCTION__
# else
#  define DEBUG_FUNC ""
# endif

/**
 * @def DEBUG
 *
 * @brief Print debug information to stdout
 *
 * @note Another name for ::DEBUG_PRINT
 */
#define DEBUG(...) do { if (ENABLE_DEBUG) { DEBUG_PRINT(__VA_ARGS__); } } while (0)

/**
 * @def DEBUG_PUTS
 *
 * @brief Print debug information to stdout using puts(), so no stack size
 *        restrictions do apply.
 */
#define DEBUG_PUTS(str) do { if (ENABLE_DEBUG) { puts(str); } } while (0)
/** @} */

/**
 * @def DEBUG_EXTRA_STACKSIZE
 *
 * @brief Extra stacksize needed when ENABLE_DEBUG==1
 */
#if ENABLE_DEBUG
#define DEBUG_EXTRA_STACKSIZE TASK_EXTRA_STACKSIZE_PRINTF
#else
#define DEBUG_EXTRA_STACKSIZE (0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* DEBUG_H */
/** @} */
