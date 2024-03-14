/*
 * Copyright (C) 2015 INRIA
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     core_util
 *
 * @{
 * @file
 * @brief       POSIX.1-2008 compliant version of the assert macro
 *
 * @author      Oliver Hahm <oliver.hahm@inria.fr>
 * @author      René Kijewski <rene.kijewski@fu-berlin.de>
 * @author      Martine Lenders <m.lenders@fu-berlin.de>
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
