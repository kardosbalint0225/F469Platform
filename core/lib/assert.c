/*
 * Original work Copyright (C) 2016 Freie Universität Berlin
 * Modified work Copyright (C) 2024 Balint Kardos
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the root
 * directory of this software component or visit
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html for more details
 *
 * The original work can be found in the RIOT-OS 2024.01 branch:
 *     RIOT-OS repository: https://github.com/RIOT-OS/RIOT/tree/2024.01-branch
 *     assert.c:           https://github.com/RIOT-OS/RIOT/blob/2024.01-branch/core/lib/assert.c
 *
 * The original author of assert.c is:
 *     Martine Lenders <mlenders@inf.fu-berlin.de>
 */

/**
 * @ingroup     core_util
 * @{
 *
 * @file        assert.c
 * @brief       POSIX.1-2008 compliant version of the assert macro
 */
#include "assert.h"
#include "panic.h"

__NORETURN void _assert_failure(const char *file, unsigned line)
{
    panic("\r\n  Assertion failed in file: ""%s on line: %u\r\n", file, line);
}


/** @} */

