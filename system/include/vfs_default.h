/*
 * Original work Copyright (C) 2022 ML!PA Consulting GmbH
 * Modified work Copyright (C) 2024 Balint Kardos
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the root
 * directory of this software component or visit
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html for more details
 *
 * The original work can be found in the RIOT-OS 2024.01 branch:
 *     RIOT-OS repository: https://github.com/RIOT-OS/RIOT/tree/2024.01-branch
 *     vfs_default.h:      https://github.com/RIOT-OS/RIOT/blob/2024.01-branch/sys/include/vfs_default.h
 *
 * The original author of vfs_default.h is:
 *     Benjamin Valentin <benjamin.valentin@ml-pa.com>
 */

/**
 * @ingroup   system_vfs
 * @brief     VFS default mount points
 *
 * @{
 * @file    vfs_default.h
 * @brief   VFS layer default definitions & mount points
 */

#ifndef VFS_DEFAULT_H
#define VFS_DEFAULT_H

#include "modules.h"
#if IS_USED(MODULE_VFS) || DOXYGEN
#include "vfs.h"
#else
/* don't try to create auto-mounts if there is no VFS module */
#define VFS_AUTO_MOUNT(type, mtd, path, idx)
#endif

#if IS_USED(MODULE_FATFS_VFS)
#include "fs/fatfs.h"
#endif
#if IS_USED(MODULE_LITTLEFS)
#include "fs/littlefs_fs.h"
#endif
#if IS_USED(MODULE_LITTLEFS2)
#include "fs/littlefs2_fs.h"
#endif
#if IS_USED(MODULE_SPIFFS)
#include "fs/spiffs_fs.h"
#endif
#if IS_USED(MODULE_LWEXT4)
#include "fs/lwext4_fs.h"
#endif
#if IS_USED(MODULE_FS_NATIVE)
#include "fs/native_fs.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Default mount point for removable storage
 */
#ifndef VFS_DEFAULT_SD
#define VFS_DEFAULT_SD(n)   "/sd" # n
#endif

/**
 * @brief   Default mount point for non-removable storage
 */
#ifndef VFS_DEFAULT_NVM
#define VFS_DEFAULT_NVM(n)  "/nvm" # n
#endif

/**
 * @brief   Default data directory
 *          This can be written to by applications
 */
#ifndef VFS_DEFAULT_DATA
#if IS_USED(MODULE_MTD_MCI) || IS_USED(MODULE_MTD_SDCARD) || \
    IS_USED(MODULE_SAM0_SDHC) || IS_USED(MODULE_MTD_SDMMC)
#define VFS_DEFAULT_DATA    VFS_DEFAULT_SD(0)
#else
#define VFS_DEFAULT_DATA    VFS_DEFAULT_NVM(0)
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* VFS_DEFAULT_H */

/** @} */
