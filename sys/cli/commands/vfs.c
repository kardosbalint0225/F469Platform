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
#include "vfs_util.h"
#include "macros/units.h"

#include <assert.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_NUM_OF_FILES    2796202ul

#define SHELL_VFS_BUFSIZE   512
static uint8_t _shell_vfs_data_buffer[SHELL_VFS_BUFSIZE];

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
    uint8_t buf[16];
    size_t nbytes = sizeof(buf);
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

    char npath[VFS_NAME_MAX];
    const char *path = embeddedCliGetToken(args, 1);
    int res = vfs_normalize_path(npath, path, strlen(path) + 1);
    if (res < 0) {
        printf("Invalid path \"%s\": %s\n", path, strerror(-res));
        return;
    }

    int fd = vfs_open(npath, O_RDONLY, 0);
    if (fd < 0) {
        printf("Error opening file \"%s\": %s\n", npath, strerror(-fd));
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
        memset(buf, 0, sizeof(buf));
        size_t line_len = (nbytes < sizeof(buf) ? nbytes : sizeof(buf));
        res = vfs_read(fd, buf, line_len);
        if (res < 0) {
            printf("Read error: %s\n", strerror(-res));
            vfs_close(fd);
            return;
        }
        else if ((size_t)res > line_len) {
            printf("BUFFER OVERRUN! %d > %lu\n", res, (unsigned long)line_len);
            vfs_close(fd);
            return;
        }
        else if (res == 0) {
            /* EOF */
            printf("-- EOF --\n");
            break;
        }

        printf("%08lx:", (unsigned long)offset);

        for (int k = 0; k < res; ++k) {
            if ((k % 2) == 0) {
                putchar(' ');
            }
            printf("%02x", buf[k]);
        }

        for (unsigned k = res; k < sizeof(buf); ++k) {
            if ((k % 2) == 0) {
                putchar(' ');
            }
            putchar(' ');
            putchar(' ');
        }

        putchar(' ');
        putchar(' ');

        for (int k = 0; k < res; ++k) {
            if (isprint(buf[k])) {
                putchar(buf[k]);
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
    return;
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
    //embeddedCliTokenizeArgs(args);

    const char *src_name = embeddedCliGetToken(args, 1);
    const char *dest_name = embeddedCliGetToken(args, 2);
    assert(src_name);
    assert(dest_name);
    printf("copy src: %s dest: %s\r\n", src_name, dest_name);

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

    uint64_t filesize = stat.st_size;
    uint64_t progress = 0;
    float percentage;
    float prev_percentage = 0;
    printf("[----------]\r\n");

    int eof = 0;
    while (eof == 0)
    {
        size_t bufspace = sizeof(_shell_vfs_data_buffer);
        size_t pos = 0;

        while (bufspace > 0)
        {
            int res = vfs_read(fd_in, &_shell_vfs_data_buffer[pos], bufspace);
            if (res < 0) {
                printf("Error reading %lu bytes @ 0x%lx in \"%s\" (%d): %s\r\n",
                       (unsigned long)bufspace, (unsigned long)pos, src_name, fd_in, strerror(-res));
                vfs_close(fd_in);
                vfs_close(fd_out);
                return;
            }

            if (res == 0) {
                /* EOF */
                eof = 1;
                break;
            }

            if (((unsigned)res) > bufspace) {
                printf("READ BUFFER OVERRUN! %d > %lu\r\n", res, (unsigned long)bufspace);
                vfs_close(fd_in);
                vfs_close(fd_out);
                return;
            }

            pos += res;
            bufspace -= res;
        }

        bufspace = pos;
        pos = 0;

        while (bufspace > 0)
        {
            int res = vfs_write(fd_out, &_shell_vfs_data_buffer[pos], bufspace);
            if (res <= 0) {
                printf("Error writing %lu bytes @ 0x%lx in \"%s\" (%d): %s\r\n",
                       (unsigned long)bufspace, (unsigned long)pos, dest_name, fd_out, strerror(-res));
                vfs_close(fd_in);
                vfs_close(fd_out);
                return;
            }

            if (((unsigned)res) > bufspace) {
                printf("WRITE BUFFER OVERRUN! %d > %lu\r\n", res, (unsigned long)bufspace);
                vfs_close(fd_in);
                vfs_close(fd_out);
                return;
            }

            progress += res;
            bufspace -= res;
        }

        percentage = 100.0f * ((float)progress / (float)filesize);
        if ((int)percentage != (int)prev_percentage)
        {
            char buf[11] = {'\0'};
            memset(buf, '-', 10);
            for (int i = 0; i < (int)percentage; i++)
            {
                buf[i] = '#';
            }
            printf("[%s]\r\n", buf);
            prev_percentage = percentage;
        }
    }

    printf("Copied: %s -> %s\r\n", src_name, dest_name);
    vfs_close(fd_in);
    vfs_close(fd_out);

    return;
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

}

/**
 * @brief  Function that is executed when the rmdir command is entered.
 *         TODO
 *
 * @param  cli (not used)
 * @param  args string containing TODO
 * @param  context (not used)
 *
 * @retval None
 */
void cli_command_rmdir(EmbeddedCli *cli, char *args, void *context)
{

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

    embeddedCliTokenizeArgs(args);
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
        embeddedCliTokenizeArgs(args);
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
            list_items_in_path(cwd);
        }
    }

}

/**
 * @brief  Prints every Mountpoints
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


