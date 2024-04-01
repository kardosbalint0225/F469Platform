/*
 * Copyright (C) 2016 Michel Rottleuthner <michel.rottleuthner@haw-hamburg.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the root
 * directory of this software component or visit
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html for more details
 *
 * The original work can be found in the RIOT-OS 2024.01 branch:
 *     RIOT-OS repository: https://github.com/RIOT-OS/RIOT/tree/2024.01-branch
 *     fatfs_diskio_mtd.h: https://github.com/RIOT-OS/RIOT/blob/2024.01-branch/pkg/fatfs/fatfs_diskio/mtd/include/fatfs_diskio_mtd.h
 *
 * The original author of fatfs_diskio_mtd.h is:
 *     Michel Rottleuthner <michel.rottleuthner@haw-hamburg.de>
 */

/**
 * @ingroup     system_fs_fatfs
 * @brief
 * @{
 * @file        fatfs_diskio_mtd.h
 * @brief       Common defines for fatfs low-level diskio defines
 */

#ifndef FATFS_DISKIO_MTD_H
#define FATFS_DISKIO_MTD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "fatfs/source/ff.h"
#include "fatfs/source/diskio.h" /* FatFs lower layer API */

#define RTC_YEAR_OFFSET   (1900)
#define FATFS_YEAR_OFFSET (1980)

#define FIXED_BLOCK_SIZE (512)

#define FATFS_DISKIO_DSTASTUS_OK (0)

#define FATFS_DISKIO_FATTIME_YEAR_OFFS (25)
#define FATFS_DISKIO_FATTIME_MON_OFFS  (21)
#define FATFS_DISKIO_FATTIME_DAY_OFFS  (16)
#define FATFS_DISKIO_FATTIME_HH_OFFS   (11)
#define FATFS_DISKIO_FATTIME_MM_OFFS   (5)

#ifdef __cplusplus
}
#endif

#endif /* FATFS_DISKIO_MTD_H */
/** @} */
