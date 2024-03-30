/*
 * Copyright (C) 2015 Kaspar Schleiser <kaspar@schleiser.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the root
 * directory of this software component or visit
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html for more details
 *
 * The original work can be found in the RIOT-OS 2024.01 branch:
 *     RIOT-OS repository: https://github.com/RIOT-OS/RIOT/tree/2024.01-branch
 *     uio.h:              https://github.com/RIOT-OS/RIOT/blob/2024.01-branch/sys/libc/include/sys/uio.h
 *
 * The original author of uio.h is:
 *     Kaspar Schleiser <kaspar@schleiser.de>
 */

/**
 * @addtogroup  posix
 * @{
 */

/**
 * @file    uio.h
 * @brief   libc header for scatter/gather I/O
 */
#ifndef SYS_UIO_H
#define SYS_UIO_H

#include <stdlib.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Structure for scatter/gather I/O.
 */
struct iovec {
    void *iov_base;     /**< Pointer to data.   */
    size_t iov_len;     /**< Length of data.    */
};

#ifdef __cplusplus
}
#endif
/** @} */
#endif /* SYS_UIO_H */
