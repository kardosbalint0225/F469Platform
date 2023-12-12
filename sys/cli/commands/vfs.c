/*
 * vfs.c
 *
 *  Created on: 2023. dec. 12.
 *      Author: Balint
 */
#include "embedded_cli.h"
#include "vfs.h"
#include "macros/units.h"

#include <stdio.h>
#include <string.h>

static char current_directory[255] = {'\0'};

static void _print_size(uint64_t size)
{
    unsigned long len;
    const char *unit;

    if (size == 0) {
        len = 0;
        unit = NULL;
    } else if ((size & (GiB(1) - 1)) == 0) {
        len = size / GiB(1);
        unit = "GiB";
    }
    else if ((size & (MiB(1) - 1)) == 0) {
        len = size / MiB(1);
        unit = "MiB";
    }
    else if ((size & (KiB(1) - 1)) == 0) {
        len = size / KiB(1);
        unit = "KiB";
    } else {
        len = size;
        unit = NULL;
    }

    if (unit) {
        printf("%8lu %s ", len, unit);
    } else {
        printf("%10lu B ", len);
    }
}

static void _print_df(vfs_DIR *dir)
{
    struct statvfs buf;
    int res = vfs_dstatvfs(dir, &buf);
    printf("%-16s ", dir->mp->mount_point);
    if (res < 0) {
        printf("statvfs failed: %s\n", strerror(-res));
        return;
    }

    _print_size(buf.f_blocks * buf.f_bsize);
    _print_size((buf.f_blocks - buf.f_bfree) * buf.f_bsize);
    _print_size(buf.f_bavail * buf.f_bsize);
    printf("%7lu%%\n", (unsigned long)(((buf.f_blocks - buf.f_bfree) * 100) / buf.f_blocks));
}

void cli_command_ls(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)context;

    if (NULL != args)
    {
        embeddedCliTokenizeArgs(args);
        //const char *path = embeddedCliGetToken(args, 1);
    }
    else
    {
        if (0 == strnlen(current_directory, sizeof(current_directory)))
        {
            puts("Mountpoint              Total         Used    Available     Use%");
            /* Iterate through all mount points */
            vfs_DIR it = { 0 };
            while (vfs_iterate_mount_dirs(&it)) {
                _print_df(&it);
            }
        }
    }

}
