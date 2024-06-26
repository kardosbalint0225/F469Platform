/*
 * Copyright (C) 2018 Kaspar Schleiser <kaspar@schleiser.de>
 *               2018 Inria
 *               2018 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the root
 * directory of this software component or visit
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html for more details
 *
 * The original work can be found in the RIOT-OS 2024.01 branch:
 *     RIOT-OS repository: https://github.com/RIOT-OS/RIOT/tree/2024.01-branch
 *     iolist.c:           https://github.com/RIOT-OS/RIOT/blob/2024.01-branch/sys/iolist/iolist.c
 *
 * The original author of iolist.c is:
 *     Kaspar Schleiser <kaspar@schleiser.de>
 */

/**
 * @ingroup     system_iolist
 * @{
 *
 * @file        iolist.c
 * @brief       iolist scatter / gather IO
 * @}
 */

#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/uio.h>

#include "iolist.h"

unsigned iolist_count(const iolist_t *iolist)
{
    unsigned count = 0;
    while (iolist) {
        count++;
        iolist = iolist->iol_next;
    }
    return count;
}

size_t iolist_size(const iolist_t *iolist)
{
    size_t result = 0;
    while (iolist) {
        result += iolist->iol_len;
        iolist = iolist->iol_next;
    }
    return result;
}

size_t iolist_to_iovec(const iolist_t *iolist, struct iovec *iov, unsigned *count)
{
    size_t bytes = 0;
    unsigned _count = 0;

    while (iolist) {
        iov->iov_base = iolist->iol_base;
        iov->iov_len = iolist->iol_len;
        bytes += iov->iov_len;
        _count++;
        iolist = iolist->iol_next;
        iov++;
    }

    *count = _count;

    return bytes;
}

ssize_t iolist_to_buffer(const iolist_t *iolist, void *buf, size_t len)
{
    char *dst = buf;

    while (iolist) {
        if (iolist->iol_len > len) {
            return -ENOBUFS;
        }
        memcpy(dst, iolist->iol_base, iolist->iol_len);
        len -= iolist->iol_len;
        dst += iolist->iol_len;
        iolist = iolist->iol_next;
    }

    return (uintptr_t)dst - (uintptr_t)buf;
}
