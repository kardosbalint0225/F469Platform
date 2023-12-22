/*
 * vfs.c
 *
 *  Created on: 2023. dec. 12.
 *      Author: Balint
 */
#include "embedded_cli.h"
#include "cli_config.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "vfs.h"
#include "macros/units.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define MAX_NUM_OF_FILES    2796202ul

static const char *get_unit(const uint64_t size, uint64_t *converted);
static void list_mountpoints(void);
static void list_files_in_directory(const char *path);

void cli_command_cd(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)context;

    if (NULL == args)
    {
        printf("\r\n  Invalid command argument\r\n");
        return;
    }

    embeddedCliTokenizeArgs(args);
    const char *path = embeddedCliGetToken(args, 1);
    assert(path);

    int res = chdir(path);
    if (res < 0) {
        printf("\r\n  The system cannot find the path specified. error: %s\r\n", strerror(-res));
    }
}

void cli_command_ls(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)context;

    if (NULL != args)
    {
        embeddedCliTokenizeArgs(args);
        const char *path = embeddedCliGetToken(args, 1);
        assert(path);

        if (0 == strncmp(path, "-mp", sizeof("-mp")))
        {
            list_mountpoints();
        }
        else
        {
            list_files_in_directory(path);
        }
    }
    else
    {
        char cwd[VFS_NAME_MAX + 1];
        if (NULL == getcwd(cwd, sizeof(cwd))) {
            printf("\r\n  Current working directory cannot be retrieved.\r\n");
            return;
        }

        if (0 == strnlen(cwd, sizeof(cwd)))
        {
            list_mountpoints();
        }
        else
        {
            list_files_in_directory(cwd);
        }
    }

}

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

static void list_files_in_directory(const char *path)
{
    char npath[CLI_CMD_BUFFER_SIZE + 1];

    int res = vfs_normalize_path(npath, path, strnlen(path, CLI_CMD_BUFFER_SIZE) + 1);
    if (res < 0) {
        printf("\r\n  Invalid path \"%s\": %s\r\n", path, strerror(-res));
        return;
    }

    vfs_DIR dir;
    res = vfs_opendir(&dir, npath);
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

        char path_name[2 * (VFS_NAME_MAX + 1)];
        snprintf(path_name, sizeof(path_name), "%s/%s", npath, entry.d_name);

        struct stat stat;
        int err = vfs_stat(path_name, &stat);
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


