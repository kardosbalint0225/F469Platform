/*
 * Copyright (C) 2016 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @{
 *
 * @file
 * @author  Martine Lenders <mlenders@inf.fu-berlin.de>
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "assert.h"
#include "cpu_conf.h"
#include "debug.h"
#include "modules.h"
#include "compiler_hints.h"

/**
 * @brief   Returns the current content of the link register (lr)
 *
 * @return  content of the link register (lr)
 */
static inline uintptr_t cpu_get_caller_pc(void)
{
    uintptr_t lr_ptr;
    __asm__ __volatile__("mov %0, lr" : "=r"(lr_ptr));
    return lr_ptr;
}

__NORETURN void _assert_failure(const char *file, unsigned line)
{
    printf("\r\n  Assertion failed in file: ""%s on line: %u\r\n", file, line);
#ifdef DEBUG_ASSERT_BREAKPOINT
    DEBUG_BREAKPOINT(1);
#endif
    abort();
}

__NORETURN void _assert_panic(void)
{
    printf("%" PRIxTXTPTR "\r\n", cpu_get_caller_pc());
#ifdef DEBUG_ASSERT_BREAKPOINT
    DEBUG_BREAKPOINT(1);
#endif
    abort();
}

/** @} */
