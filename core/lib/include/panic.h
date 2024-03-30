/*
 * Original work Copyright (C) 2014, 2015 INRIA
 * Modified work Copyright (C) 2024 Balint Kardos
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the root
 * directory of this software component or visit
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html for more details
 *
 * The original work can be found in the RIOT-OS 2024.01 branch:
 *     RIOT-OS repository: https://github.com/RIOT-OS/RIOT/tree/2024.01-branch
 *     panic.h:            https://github.com/RIOT-OS/RIOT/blob/2024.01-branch/core/lib/include/panic.h
 *
 * The original author of panic.h is:
 *     Kévin Roussel <Kevin.Roussel@inria.fr>
 */

/**
 * @ingroup     core_util
 * @{
 *
 * @file        panic.h
 * @brief       Crash handling header
 *
 * Define a panic function that allows to stop the system
 * when an unrecoverable problem has occurred.
 */

#ifndef __PANIC_H__
#define __PANIC_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __NORETURN
#ifdef __GNUC__
#define __NORETURN __attribute__((noreturn))
#else /*__GNUC__*/
#define __NORETURN
#endif /*__GNUC__*/
#endif /*__NORETURN*/

/**
 * @brief Handle an unrecoverable error by halting the system
 *
 * Detailing the failure is possible using the *message* parameter.
 *
 * @warning this function NEVER returns!
 *
 * @param[in] message       a human readable reason for the crash
 *
 * @return                  this function never returns
 * */
__NORETURN void panic(const char *message, ...);

#ifdef __cplusplus
}
#endif
#endif /* __PANIC_H__ */
/** @} */

