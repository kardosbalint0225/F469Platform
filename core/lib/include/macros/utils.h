/*
 * Copyright (C) 2023 Otto-von-Guericke-Universität Magdeburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 * The original work can be found in the RIOT-OS 2024.01 branch:
 *     RIOT-OS repository: https://github.com/RIOT-OS/RIOT/tree/2024.01-branch
 *     utils.h:            https://github.com/RIOT-OS/RIOT/blob/2024.01-branch/core/lib/include/macros/utils.h
 *
 * The original author of utils.h is:
 *     Marian Buschsieweke <marian.buschsieweke@ovgu.de>
 */

/**
 * @ingroup     core_macros
 * @{
 *
 * @file        utils.h
 * @brief       Various helper macros
 *
 */

#ifndef MACROS_UTILS_H
#define MACROS_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Concatenate the tokens @p a and @p b
 */
#define CONCAT(a, b) a ## b

/**
 * @brief   Concatenate the tokens @p a , @p b , and @p c
 */
#define CONCAT3(a, b, c) a ## b ## c

/**
 * @brief   Concatenate the tokens @p a , @p b , @p c , and @p d
 */
#define CONCAT4(a, b, c, d) a ## b ## c ## d

/* For compatibility with vendor headers, only provide MAX() and MIN() if not
 * provided. (The alternative approach of using #undef has the downside that
 * vendor header files may provide a smarter version of MAX() / MIN() that does
 * not evaluate the argument twice and rely on this).
 */
#ifndef MAX
/**
 * @brief   Get the maximum of the two parameters
 *
 * @note    This is the trivial implementation that does evaluate the arguments
 *          more than once
 */
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
/**
 * @brief   Get the minimum of the two parameters
 *
 * @note    This is the trivial implementation that does evaluate the arguments
 *          more than once
 */
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifdef __cplusplus
}
#endif

#endif /* MACROS_UTILS_H */
/** @} */
