/*
 * Copyright (C) 2021 ML!PA Consulting GmbH
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the root
 * directory of this software component or visit
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html for more details
 *
 * The original work can be found in the RIOT-OS 2024.01 branch:
 *     RIOT-OS repository: https://github.com/RIOT-OS/RIOT/tree/2024.01-branch
 *     vfs_util.h:         https://github.com/RIOT-OS/RIOT/blob/2024.01-branch/sys/include/vfs_util.h
 *
 * The original author of vfs_util.h is:
 *     Benjamin Valentin <benjamin.valentin@ml-pa.com>
 */

/**
 * @ingroup     system_vfs
 * @{
 *
 * @file        vfs_util.h
 * @brief       VFS helper functions
 */

#ifndef VFS_UTIL_H
#define VFS_UTIL_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Writes the content of a buffer to a file
 *          If the file already exists, it will be overwritten.
 *
 * @param[in]  file     Destination file path
 * @param[in]  buf      Source buffer
 * @param[in]  len      Buffer size
 *
 * @return  0 on success
 * @return  negative error from @ref vfs_open, @ref vfs_write
 */
int vfs_file_from_buffer(const char *file, const void *buf, size_t len);

/**
 * @brief   Reads the content of a file to a buffer
 *
 * @param[in]  file     Source file path
 * @param[out] buf      Destination buffer
 * @param[in]  len      Buffer size
 *
 * @return  number of bytes read on success
 * @return  -ENOSPC if the file was read successfully but is larger than
 *          the provided buffer. Only the first @p len bytes were read.
 * @return  negative error from @ref vfs_open, @ref vfs_read
 */
int vfs_file_to_buffer(const char* file, void* buf, size_t len);

#if MODULE_HASHES || DOXYGEN
/**
 * @brief   Compute the MD5 message digest of a file
 *
 *          Requires the `hashes` module.
 *
 * @param[in]  file     Source file path
 * @param[out] digest   Destination buffer, must fit @ref MD5_DIGEST_LENGTH bytes
 * @param[out] work_buf Work buffer
 * @param[in] work_buf_len  Size of the work buffer
 *
 * @return  0 on success
 * @return  negative error
 */
int vfs_file_md5(const char* file, void *digest,
                 void *work_buf, size_t work_buf_len);

/**
 * @brief   Compute the SHA1 message digest of a file
 *
 *          Requires the `hashes` module.
 *
 * @param[in]  file     Source file path
 * @param[out] digest   Destination buffer, must fit @ref SHA1_DIGEST_LENGTH bytes
 * @param[out] work_buf Work buffer
 * @param[in] work_buf_len  Size of the work buffer
 *
 * @return  0 on success
 * @return  negative error
 */
int vfs_file_sha1(const char* file, void *digest,
                  void *work_buf, size_t work_buf_len);

/**
 * @brief   Compute the SHA256 message digest of a file
 *
 *          Requires the `hashes` module.
 *
 * @param[in]  file     Source file path
 * @param[out] digest   Destination buffer, must fit @ref SHA256_DIGEST_LENGTH bytes
 * @param[out] work_buf Work buffer
 * @param[in] work_buf_len  Size of the work buffer
 *
 * @return  0 on success
 * @return  negative error
 */
int vfs_file_sha256(const char* file, void *digest,
                    void *work_buf, size_t work_buf_len);
#endif

/**
 * @brief   Checks if @p path is a file or a directory.
 *
 * This function uses @ref vfs_stat(), so if you need @ref vfs_stat() anyway,
 * you should not do double work and check it yourself.
 *
 * @param[in]   path        Path to check
 *
 * @return < 0 on FS error
 * @return 0 if @p path is a file
 * @return > 0 if @p path is a directory
 */
int vfs_is_dir(const char *path);

/**
 * @brief    Behaves like `rm -r @p root`.
 *
 * @param[in]   root        FS root directory to be deleted
 * @param[in]   path_buf    Buffer that must be able to store the longest path
 *                          of the file or directory being deleted
 * @param[in]   max_size    Size of @p path_buf
 *
 * @return < 0 on error
 * @return 0
 */
int vfs_unlink_recursive(const char *root, char *path_buf, size_t max_size);

#ifdef __cplusplus
}
#endif

#endif /* VFS_UTIL_H */
/** @} */
