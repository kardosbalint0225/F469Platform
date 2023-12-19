/*
 * vfs.c
 *
 *  Created on: 2023. dec. 12.
 *      Author: Balint
 */
#include "embedded_cli.h"
#include "cli_config.h"

#include "vfs.h"
#include "macros/units.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_NUM_OF_FILES    2796202ul

static const char * const _units[] = {
    "B", "KB", "MB", "GB", "TB"
};

static const char * const _attributes[] = {
    "<DIR>", "     "
};

enum UNIT_ID
{
    UNIT_ID_BYTE = 0,
    UNIT_ID_KILOBYTE,
    UNIT_ID_MEGABYTE,
    UNIT_ID_GIGABYTE,
    UNIT_ID_TERABYTE
};

enum ATTRIBUTE_ID
{
    ATTRIBUTE_ID_DIR = 0,
    ATTRIBUTE_ID_FILE
};

static char _cwd[FF_MAX_LFN] = {'\0'};

static const char *get_unit(const uint64_t size, uint64_t *converted);
static void print_mountpoints(void);

void cli_command_ls(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)context;

    if (NULL != args)
    {
        embeddedCliTokenizeArgs(args);
        const char *arg_path = embeddedCliGetToken(args, 1);
        assert(arg_path);

        char path[CLI_CMD_BUFFER_SIZE + 1];

        int res = vfs_normalize_path(path, arg_path, strnlen(arg_path, CLI_CMD_BUFFER_SIZE) + 1);
        if (res < 0) {
            printf("\r\n  Invalid path \"%s\": %s\r\n", arg_path, strerror(-res));
            return;
        }

        vfs_DIR dir;
        res = vfs_opendir(&dir, path);
        if (res < 0) {
            printf("\r\n  vfs_opendir error: %s\r\n", strerror(-res));
            return;
        }

        unsigned int nfiles = 0;
        unsigned int ndirs = 0;
        res = 1;

        printf("\r\n  Directory of %s\r\n\r\n", path);

        for (uint32_t i = 0; (i < MAX_NUM_OF_FILES) && (res > 0); i++)
        {
            char path_name[2 * (VFS_NAME_MAX + 1)];
            vfs_dirent_t entry;
            struct stat stat;

            res = vfs_readdir(&dir, &entry);
            if (res < 0) {
                printf("\r\n  vfs_readdir error: %s\r\n", strerror(-res));
                break;
            }

            vfs_stat(path_name, &stat);
            char mdate[11] = {'\0'};
            char mtime[6] = {'\0'};
            int attrib_id;
            char filesize[32] = {'\0'};
            if (stat.st_mode & S_IFDIR)
            {
                snprintf(filesize, sizeof(filesize), "                  ");
                attrib_id = ATTRIBUTE_ID_DIR;
                ndirs++;
            }
            else if (stat.st_mode & S_IFREG)
            {
                snprintf(filesize, sizeof(filesize), "%16lu B", stat.st_size);
                attrib_id = ATTRIBUTE_ID_FILE;
                nfiles++;
            }
            else
            {
                snprintf(filesize, sizeof(filesize), "                  ");
                attrib_id = ATTRIBUTE_ID_FILE;
            }
            strftime(mdate, sizeof(mdate), "%m/%d/%Y", localtime(&stat.st_mtim.tv_sec));
            strftime(mtime, sizeof(mtime), "%H:%M", localtime(&stat.st_mtim.tv_sec));

            printf("  %10s %5s %5s %16s %s\r\n",
                   mdate, mtime, _attributes[attrib_id], filesize, entry.d_name);
        }

        printf("\r\n%16u Dir(s)\r\n", ndirs);
        printf("%16u File(s)\r\n", nfiles);

        res = vfs_closedir(&dir);
        if (res < 0) {
            printf("vfs_closedir error: %s\n", strerror(-res));
            return;
        }
    }
    else
    {
        if (0 == strnlen(_cwd, sizeof(_cwd)))
        {
            print_mountpoints();
        }
        else
        {

        }
    }

}

static void print_mountpoints(void)
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
        return _units[UNIT_ID_GIGABYTE];
    }
    else if ((size & (MiB(1) - 1)) == 0)
    {
        *converted = size / MiB(1);
        return _units[UNIT_ID_MEGABYTE];
    }
    else if ((size & (KiB(1) - 1)) == 0)
    {
        *converted = size / KiB(1);
        return _units[UNIT_ID_KILOBYTE];
    }
    else
    {
        *converted = size;
        return _units[UNIT_ID_BYTE];
    }
}






