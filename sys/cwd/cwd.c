/*
 * cwd.c
 *
 *  Created on: 2023. dec. 22.
 *      Author: Balint
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

static char _cwd_temp[2 * (VFS_NAME_MAX + 1)];
static char _cwd[VFS_NAME_MAX] = {'\0'};
static SemaphoreHandle_t _cwd_mutex = NULL;
static StaticSemaphore_t _cwd_mutex_storage;

static void cwd_lock(void);
static void cwd_unlock(void);

static char *normalize_path(const char *path, uint8_t check);
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

int chdir(const char *__path)
{
    char *path = normalize_path(__path, 1);
    int path_len = strnlen(path, VFS_NAME_MAX);

    if (1 == path_len && '/' == path[0]) {
        cwd_lock();
        memset(_cwd, 0x00, sizeof(_cwd));
        cwd_unlock();
        return 0;
    }

    vfs_DIR dir;
    int res;
    res = vfs_opendir(&dir, path);
    if (res < 0) {
        return res;
    }

    res = vfs_closedir(&dir);
    if (res < 0) {
        return res;
    }

    cwd_lock();
    strncpy(_cwd, path, VFS_NAME_MAX);
    cwd_unlock();

    return 0;
}

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

static char *normalize_path(const char *path, uint8_t check)
{
    char *rpath = _cwd_temp;
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
    memset(rpath, 0x00, sizeof(_cwd_temp));

    if (*path != '/') {

        // It's a relative path, so prepend the current working directory
        if (!strncpy(rpath, _cwd, VFS_NAME_MAX)) {
            return NULL;
        }

        // Append / if the current working directory doesn't end with /
        if (*(rpath + strlen(rpath) - 1) != '/') {
            strcat(rpath, "/");
        }

        // Append initial path
        strcat(rpath, path);
    } else {
        strcpy(rpath, path);
    }

    cpath = rpath;
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
            cpath = dot_dot(rpath, cpath);

            is_dot = 0;
            is_dot_dot = 0;
            maybe_is_dot = 0;
            maybe_is_dot_dot = 0;

            continue;
        } else if (is_dot) {
            cpath = dot(rpath, cpath);

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
        cpath = dot_dot(rpath, cpath);
    } else if (is_dot || maybe_is_dot) {
        cpath = dot(rpath, cpath);
    }

    // Normalize //
    cpath = rpath;
    while (*cpath) {
        if (*cpath == '/') {
            is_slash_slash = is_slash;
            is_slash = 1;
        } else {
            is_slash = 0;
            is_slash_slash = 0;
        }

        if (is_slash_slash) {
            cpath = slash_slash(rpath, cpath);

            is_slash = 1;
            is_slash_slash = 0;

            continue;
        }
        cpath++;
    }

    // End case
    if (is_slash_slash) {
        cpath = slash_slash(rpath, cpath);
    }

    // A normalized path must never end by /, except if it is the
    // root folder
    cpath = (strlen(rpath) > 1)?(rpath + strlen(rpath) - 1):NULL;
    if (cpath && (*cpath == '/')) {
        *cpath = '\0';
    }

    // Check that fits in PATH_MAX
    if (strlen(rpath) > VFS_NAME_MAX) {
        return NULL;
    }

    char *tmp = strdup(rpath);
    if (tmp) {
        rpath = tmp;
    }

    return rpath;
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



