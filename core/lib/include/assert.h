/*
 * Original work Copyright (C) 2015 INRIA
 *                             2016 Freie Universität Berlin
 * Modified work Copyright (C) 2024 Balint Kardos
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the root
 * directory of this software component or visit
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html for more details
 *
 * The original work can be found in the RIOT-OS 2024.01 branch:
 *     RIOT-OS repository: https://github.com/RIOT-OS/RIOT/tree/2024.01-branch
 *     assert.h:           https://github.com/RIOT-OS/RIOT/blob/2024.01-branch/core/lib/include/assert.h
 *
 * The original authors of assert.h are:
 *     Oliver Hahm <oliver.hahm@inria.fr>
 *     René Kijewski <rene.kijewski@fu-berlin.de>
 *     Martine Lenders <m.lenders@fu-berlin.de>
 */

/**
 * @ingroup     core_util
 * @{
 * @file        assert.h
 * @brief       POSIX.1-2008 compliant version of the assert macro
 */

#ifndef ASSERT_H
#define ASSERT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define _likely(x)      __builtin_expect((uintptr_t)(x), 1)

#ifndef __NORETURN
#ifdef __GNUC__
#define __NORETURN __attribute__((noreturn))
#else /*__GNUC__*/
#define __NORETURN
#endif /*__GNUC__*/
#endif /*__NORETURN*/

#ifdef NDEBUG
#define assert(ignore)((void)0)
#else
/**
 * @brief   Function to handle failed assertion
 *
 * @warning this function **NEVER** returns!
 *
 * @param[in] file  The file name of the file the assertion failed in
 * @param[in] line  The code line of @p file the assertion failed in
 */
__NORETURN void _assert_failure(const char *file, unsigned line);
/**
 * @brief    abort the program if assertion is false
 *
 * If the macro NDEBUG was defined at the moment <assert.h> was last included,
 * the macro assert() generates no code, and hence does nothing at all.
 */
#define assert(cond) (_likely(cond) ? (void)0 :  _assert_failure(__FILE__, __LINE__))
#endif

#if !defined __cplusplus
#if __STDC_VERSION__ >= 201112L
/**
 * @brief c11 static_assert() macro
 */
#define static_assert(...) _Static_assert(__VA_ARGS__)
#else
/**
 * @brief static_assert for c-version < c11
 *
 * Generates a division by zero compile error when cond is false
 */
#define static_assert(cond, ...) \
    { enum { static_assert_failed_on_div_by_0 = 1 / (!!(cond)) }; }
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* ASSERT_H */
/** @} */
