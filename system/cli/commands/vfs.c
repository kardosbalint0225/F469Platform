/*
 * Original work Copyright (C) 2016 Eistec AB
 * Modified work Copyright (C) 2024 Balint Kardos
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the root
 * directory of this software component or visit
 * https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html for more details
 *
 * The original work can be found in the RIOT-OS 2024.01 branch:
 *     RIOT-OS repository: https://github.com/RIOT-OS/RIOT/tree/2024.01-branch
 *     vfs.c:              https://github.com/RIOT-OS/RIOT/blob/2024.01-branch/sys/shell/cmds/vfs.c
 *
 * The original author of vfs.c is:
 *     Joakim Nohlgård <joakim.nohlgard@eistec.se>
 */

/**
 * @ingroup     system_cli
 * @{
 *
 * @file        vfs.c
 * @brief       CLI commands for the VFS module
 *
 * @}
 */

#include "embedded_cli.h"
#include "cli_config.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "vfs.h"
#include "vfs_util.h"
#include "macros/units.h"

#include <assert.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>

#define MAX_NUM_OF_FILES    2796202ul

static char _work_area[1024ul];
static char _path[VFS_NAME_MAX + 1];

static const char *get_unit(const uint64_t size, uint64_t *converted);
static void list_mountpoints(void);
static void list_items_in_path(const char *path);

/**
 * @brief  Function that is executed when the r command is entered.
 *         TODO
 *
 * @param  cli (not used)
 * @param  args string containing TODO
 * @param  context (not used)
 *
 * @retval None
 */
void cli_command_r(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)context;

    size_t nbytes = sizeof(_work_area);
    off_t offset = 0;

    int argc = embeddedCliGetTokenCount(args);
    if (argc < 1) {
        printf("vfs read: missing file name\r\n");
        return;
    }

    if (argc > 1) {
        nbytes = atoi(embeddedCliGetToken(args, 2));
    }

    if (argc > 2) {
        offset = atoi(embeddedCliGetToken(args, 3));
    }

    const char *path = embeddedCliGetToken(args, 1);
    int res = vfs_normalize_path(_path, path, strlen(path) + 1);
    if (res < 0) {
        printf("Invalid path \"%s\": %s\n", path, strerror(-res));
        return;
    }

    int fd = vfs_open(_path, O_RDONLY, 0);
    if (fd < 0) {
        printf("Error opening file \"%s\": %s\n", _path, strerror(-fd));
        return;
    }

    res = vfs_lseek(fd, offset, SEEK_SET);
    if (res < 0) {
        printf("Seek error: %s\n", strerror(-res));
        vfs_close(fd);
        return;
    }

    while (nbytes > 0)
    {
        memset(_work_area, 0, sizeof(_work_area));
        size_t line_len = (nbytes < 16 ? nbytes : 16);
        res = vfs_read(fd, _work_area, line_len);
        if (res < 0) {
            printf("Read error: %s\n", strerror(-res));
            vfs_close(fd);
            return;
        }

        if (res == 0) {
            /* EOF */
            printf("-- EOF --\n");
            break;
        }

        printf("  %08lx:", (unsigned long)offset);

        for (int k = 0; k < res; ++k) {
            if ((k % 2) == 0) {
                putchar(' ');
            }
            printf("%02x", _work_area[k]);
        }

        for (unsigned k = res; k < 16; ++k) {
            if ((k % 2) == 0) {
                putchar(' ');
            }
            putchar(' ');
            putchar(' ');
        }

        putchar(' ');
        putchar(' ');

        for (int k = 0; k < res; ++k) {
            if (isprint((int)_work_area[k])) {
                putchar(_work_area[k]);
            }
            else {
                putchar('.');
            }
        }

        puts("\r");
        offset += res;
        nbytes -= res;
    }

    vfs_close(fd);
}

/**
 * @brief  Function that is executed when the cp command is entered.
 *         TODO
 *
 * @param  cli (not used)
 * @param  args string containing TODO
 * @param  context (not used)
 *
 * @retval None
 */
void cli_command_cp(EmbeddedCli *cli, char *args, void *context)
{
    if (NULL == args)
    {
        printf("Invalid command argument.\r\n");
        return;
    }

    const char *src_name = embeddedCliGetToken(args, 1);
    const char *dest_name = embeddedCliGetToken(args, 2);
    assert(src_name);
    assert(dest_name);

    printf("  copy src: %s dest: %s\r\n", src_name, dest_name);

    int fd_in = vfs_open(src_name, O_RDONLY, 0);
    if (fd_in < 0) {
        printf("Error opening file for reading \"%s\": %s\r\n", src_name, strerror(-fd_in));
        return;
    }

    struct stat stat;
    int err = vfs_fstat(fd_in, &stat);
    if (err < 0) {
        printf("vfs_fstat error : %s\r\n", strerror(-err));
        vfs_close(fd_in);
        return;
    }

    int fd_out = vfs_open(dest_name, O_WRONLY | O_TRUNC | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd_out < 0) {
        printf("Error opening file for writing \"%s\": %s\r\n", dest_name, strerror(-fd_out));
        vfs_close(fd_in);
        return;
    }

    char tbuf[10] = {'\0'};
    struct timeval tv;
    struct tm timeinfo;

    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &timeinfo);
    strftime(tbuf, sizeof(tbuf), "%T", &timeinfo);
    printf("  Started at  %s\r\n", tbuf);

    uint64_t bytes_copied = 0;
    uint64_t file_size = (uint64_t)stat.st_size;
    int progress = 0;
    int bytes_copied_between = 0;
    printf("    [--------------------------------------------------] %3d %%\r\n\b\r", progress);
    TickType_t tstart = xTaskGetTickCount();

    int eof = 0;
    while (eof == 0)
    {
        int bytes_read = vfs_read(fd_in, _work_area, sizeof(_work_area));
        if (bytes_read < 0) {
            printf("  Error reading in \"%s\" (%d): %s\r\n", src_name, fd_in, strerror(-bytes_read));
            vfs_close(fd_in);
            vfs_close(fd_out);
            return;
        }

        if (bytes_read == 0) {
            /* EOF */
            eof = 1;
        }

        int bytes_written = vfs_write(fd_out, _work_area, bytes_read);
        if (bytes_written < 0) {
            printf("Error writing in \"%s\" (%d): %s\r\n", dest_name, fd_out, strerror(-bytes_written));
            vfs_close(fd_in);
            vfs_close(fd_out);
            return;
        }

        bytes_copied_between += bytes_written;
        bytes_copied += bytes_written;
        const double percent = 100.0f * ((double)bytes_copied / (double)file_size);
        const int diff = (int)percent - progress;
        if (diff > 2) {
            TickType_t tend = xTaskGetTickCount();
            const float tdiff = (float)(tend - tstart) / 1000.0f;
            const float bps = (float)bytes_copied_between / tdiff;
            bytes_copied_between = 0;

            progress = (int)percent;
            char buf[51] = {'\0'};
            for (int i = 0; i < 50; i++) {
                if (i < (percent / 2)) {
                    buf[i] = '#';
                } else {
                    buf[i] = '-';
                }
            }
            printf("  [%50s] %3d %%  | %10.3f KB/s\r\n\b\r", buf, progress, bps / 1024);
            tstart = xTaskGetTickCount();
        }
    }

    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &timeinfo);
    strftime(tbuf, sizeof(tbuf), "%T", &timeinfo);
    printf("\r\n  Finished at %s\r\n", tbuf);

    printf("  Copied: %s -> %s\r\n", src_name, dest_name);
    vfs_close(fd_in);
    vfs_close(fd_out);
}

/**
 * @brief  Function that is executed when the mv command is entered.
 *         TODO
 *
 * @param  cli (not used)
 * @param  args string containing TODO
 * @param  context (not used)
 *
 * @retval None
 */
void cli_command_mv(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)args;
    (void)context;

    assert(0);
}

/**
 * @brief  Function that is executed when the rm command is entered.
 *         TODO
 *
 * @param  cli (not used)
 * @param  args string containing TODO
 * @param  context (not used)
 *
 * @retval None
 */
void cli_command_rm(EmbeddedCli *cli, char *args, void *context)
{
    int argc = embeddedCliGetTokenCount(args);
    if (argc < 1) {
        printf("  Invalid command argument.\r\n");
        return;
    }

    bool recursive = !strncmp(embeddedCliGetToken(args, 1), "-r", CLI_CMD_BUFFER_SIZE);
    if (recursive && argc < 2) {
        printf("  Invalid command argument.\r\n");
        return;
    }

    const char *rm_name = recursive ? embeddedCliGetToken(args, 2) : embeddedCliGetToken(args, 1);
    printf("  unlink: %s\n", rm_name);

    int res;
    if (recursive) {
        res = vfs_unlink_recursive(rm_name, _path, sizeof(_path));
    } else {
        res = vfs_unlink(rm_name);
    }

    if (res < 0) {
        printf("  rm error: %s\n", strerror(-res));
    }
}

/**
 * @brief  Function that is executed when the mkdir command is entered.
 *         TODO
 *
 * @param  cli (not used)
 * @param  args string containing TODO
 * @param  context (not used)
 *
 * @retval None
 */
void cli_command_mkdir(EmbeddedCli *cli, char *args, void *context)
{
    int argc = embeddedCliGetTokenCount(args);
    if (argc < 1) {
        printf("  Invalid command argument.\r\n");
        return;
    }
    const char *dir_name = embeddedCliGetToken(args, 1);
    assert(dir_name);
    printf("  mkdir: %s\n", dir_name);

    int res = vfs_mkdir(dir_name, 0);
    if (res < 0) {
        printf("  mkdir error: %s\n", strerror(-res));
    }
}

/**
 * @brief  Function that is executed when the cd command is entered.
 *         Changes the current working directory to the given <path>
 *
 * @param  cli (not used)
 * @param  args string containing the absolute or relative path
 * @param  context (not used)
 *
 * @retval None
 */
void cli_command_cd(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)context;

    if (NULL == args)
    {
        printf("\r\n  Invalid command argument\r\n");
        return;
    }

    const char *path = embeddedCliGetToken(args, 1);
    assert(path);

    int res = chdir(path);
    if (res < 0) {
        printf("\r\n  The system cannot find the path specified. error: %s\r\n", strerror(-res));
    }
}

/**
 * @brief  Function that is executed when the ls command is entered.
 *         Lists the files and folders in the given directory. If the argument
 *         is NULL then the CWD is used
 *
 * @param  cli (not used)
 * @param  args string containing the absolute or relative path or NULL
 * @param  context (not used)
 *
 * @retval None
 */
void cli_command_ls(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)context;

    if (NULL != args)
    {
        const char *path = embeddedCliGetToken(args, 1);
        assert(path);

        if (0 == strncmp(path, "-mp", sizeof("-mp")))
        {
            list_mountpoints();
        }
        else
        {
            list_items_in_path(path);
        }
    }
    else
    {
        if (NULL == getcwd(_path, sizeof(_path))) {
            printf("\r\n  Current working directory cannot be retrieved.\r\n");
            return;
        }

        if (0 == strnlen(_path, sizeof(_path)))
        {
            list_mountpoints();
        }
        else
        {
            list_items_in_path(_path);
        }
    }

}

/**
 * @brief  Prints every Mount-point
 *
 * @param  None
 *
 * @retval None
 */
static void list_mountpoints(void)
{
    printf("\r\n  Mountpoint |     Total     |     Used      |   Available   |     Use %%\r\n");
    printf("  -----------+---------------+---------------+---------------+-----------\r\n");
    /* Iterate through all mount points */
    vfs_DIR dir = { 0 };

    while (vfs_iterate_mount_dirs(&dir))
    {
        struct statvfs buf;
        int res = vfs_dstatvfs(&dir, &buf);

        if (res < 0) {
            printf("statvfs failed: %s\n", strerror(-res));
            return;
        }

        const uint64_t total = buf.f_blocks * buf.f_bsize;
        const uint64_t used = (buf.f_blocks - buf.f_bfree) * buf.f_bsize;
        const uint64_t free = buf.f_bavail * buf.f_bsize;

        uint64_t total_conv;
        uint64_t used_conv;
        uint64_t free_conv;
        const char *total_unit = get_unit(total, &total_conv);
        const char *used_unit = get_unit(used, &used_conv);
        const char *free_unit = get_unit(free, &free_conv);

        const uint32_t use = (uint32_t)(((buf.f_blocks - buf.f_bfree) * 100) / buf.f_blocks);

        printf("  %-10s | %10llu %2s | %10llu %2s | %10llu %2s | %7lu %%\r\n",
               dir.mp->mount_point, total_conv, total_unit, used_conv, used_unit, free_conv, free_unit, use);
    }
}

/**
 * @brief  Converts the given size to the nearest byte unit
 *         and returns the unit in string format
 *
 * @param  size the size to be converted
 * @param  converted pointer where the result of the conversion can be stored
 *
 * @retval "B", "KB", "MB", "GB" or "TB" according to the conversion
 * @retvan NULL if size is 0
 */
static const char *get_unit(const uint64_t size, uint64_t *converted)
{
    assert(converted);

    if (size == 0)
    {
        *converted = size;
        return NULL;
    }
    else if ((size & (GiB(1) - 1)) == 0)
    {
        *converted = size / GiB(1);
        return "GB";
    }
    else if ((size & (MiB(1) - 1)) == 0)
    {
        *converted = size / MiB(1);
        return "MB";
    }
    else if ((size & (KiB(1) - 1)) == 0)
    {
        *converted = size / KiB(1);
        return "KB";
    }
    else
    {
        *converted = size;
        return "B";
    }
}

/**
 * @brief  Lists every file and folder in the given path
 *
 * @param  path pointer where the path to be listed is stored
 *
 * @retval None
 */
static void list_items_in_path(const char *path)
{
    //char npath[CLI_CMD_BUFFER_SIZE + 1];

    int res = vfs_normalize_path(_path, path, strnlen(path, CLI_CMD_BUFFER_SIZE) + 1);
    if (res < 0) {
        printf("\r\n  Invalid path \"%s\": %s\r\n", path, strerror(-res));
        return;
    }

    vfs_DIR dir;
    res = vfs_opendir(&dir, _path);
    if (res < 0) {
        printf("\r\n  vfs_opendir error: %s\r\n", strerror(-res));
        return;
    }

    unsigned int nfiles = 0;
    unsigned int ndirs = 0;

    printf("\r\n  Directory of %s\r\n\r\n", path);

    for (uint32_t i = 0; i < MAX_NUM_OF_FILES; i++)
    {
        vfs_dirent_t entry;
        res = vfs_readdir(&dir, &entry);
        if (res < 0) {
            printf("\r\n  vfs_readdir error: %s\r\n", strerror(-res));
            break;
        }
        if (res == 0) {
            break;
        }

        memset(_work_area, 0x00, (2 * (VFS_NAME_MAX + 1)));
        snprintf(_work_area, sizeof(_work_area), "%s/%s", _path, entry.d_name);

        struct stat stat;
        int err = vfs_stat(_work_area, &stat);
        if (err < 0) {
            printf("\r\n  vfs_stat error: %s\r\n", strerror(-err));
            break;
        }

        struct tm tbuf;
        char mdate[11] = {'\0'};
        strftime(mdate, sizeof(mdate), "%m/%d/%Y", localtime_r(&stat.st_mtim.tv_sec, &tbuf));
        char mtime[6] = {'\0'};
        strftime(mtime, sizeof(mtime), "%H:%M", localtime_r(&stat.st_mtim.tv_sec, &tbuf));

        char filesize[17] = {'\0'};
        if (stat.st_mode & S_IFDIR)
        {
            snprintf(filesize, sizeof(filesize), "%-16s", "<DIR>");
            ndirs++;
        }
        else if (stat.st_mode & S_IFREG)
        {
            snprintf(filesize, sizeof(filesize), "%16lu", stat.st_size);
            nfiles++;
        }
        else
        {
            snprintf(filesize, sizeof(filesize), " ");
        }


        printf("%10s  %5s  %16s %s\r\n",
               mdate, mtime, filesize, entry.d_name);
    }

    printf("\r\n%16u Dir(s)\r\n", ndirs);
    printf("%16u File(s)\r\n", nfiles);

    res = vfs_closedir(&dir);
    if (res < 0) {
        printf("vfs_closedir error: %s\n", strerror(-res));
        return;
    }
}


