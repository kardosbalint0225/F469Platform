/*
 * Original work Copyright (C) 2015 - 2020, IBEROXARXA SERVICIOS INTEGRALES, S.L.
 *                             2015 - 2020, Jaume Olivé Petrus (jolive@whitecatboard.org)
 * Modified work Copyright (C) 2024 Balint Kardos
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *     * Redistributions in binary form must retain all copyright notices printed
 *       to any local or remote output device. This include any reference to
 *       Lua RTOS, whitecatboard.org, Lua, and other copyright notices that may
 *       appear in the future.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/**
 * @ingroup    system_cwd
 * @{
 * @file       cwd.c
 * @brief      Implementation of Current Working Directory (CWD),
 *             chdir and getcwd functions
 *
 * @note       normalize_path, dot, dot_dot and slash_slash functions are derived from the
 *             mount.c file of the Lua-RTOS-ESP32 repository:
 *
 * Link to Lua-RTOS-ESP32:
 *     https://github.com/whitecatboard/Lua-RTOS-ESP32/tree/master
 *
 * Link to mount.c:
 *     https://github.com/whitecatboard/Lua-RTOS-ESP32/blob/master/components/sys/sys/mount.c
 */
#include "cwd.h"
#include "vfs.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static char _path[VFS_NAME_MAX + 1];
static char _cwd_work_area[2 * (VFS_NAME_MAX + 1)];
static char _cwd[VFS_NAME_MAX + 1];
static SemaphoreHandle_t _cwd_mutex = NULL;
static StaticSemaphore_t _cwd_mutex_storage;

static void cwd_lock(void);
static void cwd_unlock(void);

static int normalize_path(char *to, size_t to_len, const char *from, size_t from_len, char *work_area, size_t work_area_len);
static char *dot(char *rpath, char *cpath);
static char *dot_dot(char *rpath, char *cpath);
static char *slash_slash(char *rpath, char *cpath);

void cwd_init(void)
{
    _cwd_mutex = xSemaphoreCreateRecursiveMutexStatic(&_cwd_mutex_storage);
    assert(_cwd_mutex);
    memset(_cwd, 0x00, sizeof(_cwd));
}

void cwd_deinit(void)
{
    vSemaphoreDelete(_cwd_mutex);
    _cwd_mutex = NULL;
}

/**
 * @brief  Change the current working directory to the given path
 *
 * @param  __path pointer where the path is stored
 *
 * @return 0 if changing the current working directory was successful
 * @return -ENAMETOOLONG if the path combined with cwd is larger than VFS_NAME_MAX
 */
int chdir(const char *__path)
{
    int res = normalize_path(_path, sizeof(_path),
                             __path, strnlen(__path, VFS_NAME_MAX),
                             _cwd_work_area, sizeof(_cwd_work_area));
    if (res < 0) {
        return res;
    }

    int path_len = strnlen(_path, VFS_NAME_MAX);

    if (1 == path_len && '/' == _path[0]) {
        cwd_lock();
        memset(_cwd, 0x00, sizeof(_cwd));
        cwd_unlock();
        return 0;
    }

    vfs_DIR dir;
    res = vfs_opendir(&dir, _path);
    if (res < 0) {
        return res;
    }

    res = vfs_closedir(&dir);
    if (res < 0) {
        return res;
    }

    cwd_lock();
    strncpy(_cwd, _path, VFS_NAME_MAX);
    cwd_unlock();

    return 0;
}

/**
 * @brief  Get the current working directory
 *
 * @param  __buf pointer where the CWD can be stored
 * @param  __size the size of the given __buf
 *
 * @return the address of __bug if storing the current working directory was successful
 * @return NULL if __buf is NULL or the size of __buf is smaller than length of the cwd
 */
char *getcwd(char *__buf, size_t __size)
{
    if ((NULL == __buf) || (__size < sizeof(_cwd))) {
        return NULL;
    }

    cwd_lock();
    char *dest = strncpy(__buf, _cwd, __size);
    cwd_unlock();

    return dest;
}

/**
 * @brief  Locks the CWD mutex
 */
static void cwd_lock(void)
{
    xSemaphoreTakeRecursive(_cwd_mutex, portMAX_DELAY);
}

/**
 * @brief  Unlocks the CWD mutex
 */
static void cwd_unlock(void)
{
    xSemaphoreGiveRecursive(_cwd_mutex);
}

/**
 * @brief  Normalize (absolutize) the given path
 *
 * @param  to pointer where the normalized path can be stored
 * @param  length of the to buffer
 * @param  from pointer where the path to be normalized is stored
 * @param  from_len the length of the from path string
 * @param  work_area working area for the normalization should be (from_len + VFS_NAME_MAX + 2) long
 * @param  work_area_len the size of the working area
 *
 * @return 0 if the normalization was successful
 * @return -ENAMETOOLONG if the path combined with cwd is larger than VFS_NAME_MAX
 */
static int normalize_path(char *to, size_t to_len, const char *from, size_t from_len, char *work_area, size_t work_area_len)
{
    char *cpath;
    int maybe_is_dot = 0;
    int maybe_is_dot_dot = 0;
    int is_dot = 0;
    int is_dot_dot = 0;
    int is_slash = 0;
    int is_slash_slash = 0;

    // We need as many characters as the initial path + PATH_MAX + 1
    // to prepend the current directory + 1 additional character for
    // the end of the string.
    //
    // We can use strcpy / strcat functions because destination string has enough
    // allocated space.
    if (((from_len + VFS_NAME_MAX + 2) > work_area_len) || (to_len < VFS_NAME_MAX)) {
        return -ENAMETOOLONG;
    }

    memset(work_area, 0x00, work_area_len);

    if (*from != '/') {

        // It's a relative path, so prepend the current working directory
        if (NULL == getcwd(work_area, work_area_len)) {
            return -ENAMETOOLONG;
        }

        // Append / if the current working directory doesn't end with /
        if (*(work_area + strnlen(work_area, work_area_len) - 1) != '/') {
            strcat(work_area, "/");
        }

    }

    // Append initial path
    strcat(work_area, from);

    cpath = work_area;
    while (*cpath) {
        if (*cpath == '.') {
            maybe_is_dot_dot = is_slash && maybe_is_dot;
            maybe_is_dot = is_slash && !maybe_is_dot_dot;
        } else if (*cpath == '/') {
            is_dot_dot = maybe_is_dot_dot;
            is_dot = maybe_is_dot && !is_dot_dot;
            is_slash = 1;
        } else {
            maybe_is_dot = 0;
            maybe_is_dot_dot = 0;
            is_slash = 0;
        }

        if (is_dot_dot) {
            cpath = dot_dot(work_area, cpath);

            is_dot = 0;
            is_dot_dot = 0;
            maybe_is_dot = 0;
            maybe_is_dot_dot = 0;

            continue;
        } else if (is_dot) {
            cpath = dot(work_area, cpath);

            is_dot = 0;
            is_dot_dot = 0;
            maybe_is_dot = 0;
            maybe_is_dot_dot = 0;

            continue;
        }

        cpath++;
    }

    // End case
    is_dot_dot = maybe_is_dot_dot;
    is_dot = maybe_is_dot && !is_dot_dot;

    if (is_dot_dot) {
        cpath = dot_dot(work_area, cpath);
    } else if (is_dot || maybe_is_dot) {
        cpath = dot(work_area, cpath);
    }

    // Normalize //
    cpath = work_area;
    while (*cpath) {
        if (*cpath == '/') {
            is_slash_slash = is_slash;
            is_slash = 1;
        } else {
            is_slash = 0;
            is_slash_slash = 0;
        }

        if (is_slash_slash) {
            cpath = slash_slash(work_area, cpath);

            is_slash = 1;
            is_slash_slash = 0;

            continue;
        }
        cpath++;
    }

    // End case
    if (is_slash_slash) {
        cpath = slash_slash(work_area, cpath);
    }

    // A normalized path must never end by /, except if it is the
    // root folder
    cpath = (strnlen(work_area, work_area_len) > 1) ? (work_area + strnlen(work_area, work_area_len) - 1) : NULL;
    if (cpath && (*cpath == '/')) {
        *cpath = '\0';
    }

    // Check that fits in PATH_MAX
    if (strnlen(work_area, work_area_len) > VFS_NAME_MAX) {
        return -ENAMETOOLONG;
    }

    strncpy(to, work_area, to_len);

    return 0;
}

static char *dot_dot(char *rpath, char *cpath)
{
    char *last;

    if (cpath >= rpath + strlen(rpath)) {
        last = rpath + strlen(rpath);
    } else {
        last = cpath + 1;
    }

    while ((cpath - 1 >= rpath) && (*--cpath != '/'));
    while ((cpath - 1 >= rpath) && (*--cpath != '/'));

    char *tpath = ++cpath;
    while (*last) {
        *tpath++ = *last++;
    }
    *tpath = '\0';

    return cpath;
}

static char *dot(char *rpath, char *cpath)
{
    char *last;

    if (cpath >= rpath + strlen(rpath)) {
        last = rpath + strlen(rpath);
    } else {
        last = cpath + 1;
    }

    while ((cpath - 1 >= rpath) && (*--cpath != '/'));

    char *tpath = ++cpath;
    while (*last) {
        *tpath++ = *last++;
    }
    *tpath = '\0';

    return cpath;
}

static char *slash_slash(char *rpath, char *cpath)
{
    char *last = cpath + 1;

    while ((cpath - 1 >= rpath) && (*--cpath != '/'));

    char *tpath = ++cpath;
    while (*last) {
        *tpath++ = *last++;
    }
    *tpath = '\0';

    return cpath;
}
/** @} */


