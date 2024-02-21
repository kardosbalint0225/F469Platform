/**
 ******************************************************************************
 * @file      syscalls.c
 *
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
#include <unistd.h>
#include <reent.h>
#include <errno.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/times.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "vfs.h"
#include "rtc.h"

char *__env[1] = { 0 };
char **environ = __env;

/**
 * Pointer to the current high watermark of the heap usage
 */
static uint8_t *__sbrk_heap_end = NULL;
extern uint8_t _end; /* Symbol defined in the linker script */
extern uint8_t _estack; /* Symbol defined in the linker script */
extern uint32_t _Min_Stack_Size; /* Symbol defined in the linker script */

/**
 * @brief _sbrk() allocates memory to the newlib heap and is used by malloc
 *        and others from the C library
 *
 * @verbatim
 * ############################################################################
 * #  .data  #  .bss  #       newlib heap       #          MSP stack          #
 * #         #        #                         # Reserved by _Min_Stack_Size #
 * ############################################################################
 * ^-- RAM start      ^-- _end                             _estack, RAM end --^
 * @endverbatim
 *
 * This implementation starts allocating at the '_end' linker symbol
 * The '_Min_Stack_Size' linker symbol reserves a memory for the MSP stack
 * The implementation considers '_estack' linker symbol to be RAM end
 * NOTE: If the MSP stack, at any point during execution, grows larger than the
 * reserved size, please increase the '_Min_Stack_Size'.
 *
 * @param  ptr Pointer to the global data block, which holds errno
 * @param  incr Memory size
 * @return Pointer to allocated memory
 */
void *_sbrk_r(struct _reent *ptr, ptrdiff_t incr)
{
    const uint32_t stack_limit = (uint32_t)&_estack - (uint32_t)&_Min_Stack_Size;
    const uint8_t *max_heap = (uint8_t *)stack_limit;
    uint8_t *prev_heap_end;

    /* Initialize heap end at first call */
    if (NULL == __sbrk_heap_end)
    {
        __sbrk_heap_end = &_end;
    }

    /* Protect heap from growing into the reserved MSP stack */
    if (__sbrk_heap_end + incr > max_heap)
    {
        ptr->_errno = ENOMEM;
        return (void *)-1;
    }

    prev_heap_end = __sbrk_heap_end;
    __sbrk_heap_end += incr;

    return (void *)prev_heap_end;
}

/**
 * @brief  Fork execution into two threads
 *
 * @note   Not supported.
 *
 * @param  ptr Pointer to the global data block, which holds errno
 *
 * @return PID of child process if parent, 0 if child, negative value on failure.
 */
int _fork_r(struct _reent *ptr)
{
    ptr->_errno = ENOTSUP;
    return -1;
}

/**
 * @brief  Load and execute an executable given a path
 *
 * @note   Not supported.
 *
 * @param  ptr  Pointer to the global data block, which holds errno
 * @param  name Filename of the executable
 * @param  argv Array of pointers to arguments
 * @param  env  Array of pointers to environment variables
 *
 * @return 0 on success or a negative value on error.
 */
int _execve_r(struct _reent *ptr, const char *name, char *const argv[], char *const env[])
{
    (void)name;
    (void)argv;
    (void)env;
    ptr->_errno = ENOTSUP;
    return -1;
}

/**
 * @brief  Send a signal to a PID
 *
 * @note   Not supported.
 *
 * @param  ptr Pointer to the global data block, which holds errno
 * @param  pid The PID of the process
 * @param  sig The signal to send
 *
 * @return 0 on success or a negative value on error.
 */
int _kill_r(struct _reent *ptr, int pid, int sig)
{
    (void)pid;
    (void)sig;
    ptr->_errno = ENOTSUP;
    return -1;
}

/**
 * @brief  Wait for a child process
 *
 * @note   Not supported.
 *
 * @param  ptr    Pointer to the global data block, which holds errno
 * @param  status Status of the wait operation
 *
 * @return 0 on success or a negative value on error.
 */
int _wait_r(struct _reent *ptr, int *status)
{
    (void)status;
    ptr->_errno = ENOTSUP;
    return -1;
}

/**
 * @brief  Return the PID of the current thread
 *
 * @param  ptr Pointer to the global data block, which holds errno
 *
 * @return The PID on success or a negative value on error.
 */
int _getpid_r(struct _reent *ptr)
{
    (void)ptr;

    return 1;
}

/**
 * @brief Exit a program without cleaning up files
 *
 * If your system doesn't provide this, it is best to avoid linking with subroutines that
 * require it (exit, system).
 *
 * @param status the exit code, 0 for all OK, >0 for not OK
 */
void _exit(int status)
{
    (void)status;

    while (1)
    {
        HAL_GPIO_TogglePin(GPIOD, GPIO_PIN_5);  //TODO: Move to GPIO layer
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * @brief  Query whether output stream is a terminal
 *
 * @param  ptr Pointer to the global data block, which holds errno
 * @param  fd  File handle
 *
 * @return 1 if the file is a TTY, or 0 if not.
 */
int _isatty_r(struct _reent *ptr, int fd)
{
    (void)ptr;

    int retv = 0;

    if (STDIN_FILENO == fd || STDOUT_FILENO == fd || STDERR_FILENO == fd)
    {
        retv = 1;
    }

    return retv;
}

/**
 * @brief  Open a file
 *
 * This is a wrapper around @c vfs_open
 *
 * @param  ptr   pointer to reent structure
 * @param  name  file name to open
 * @param  flags flags, see man 3p open
 * @param  mode  mode, file creation mode if the file is created when opening
 *
 * @return fd number (>= 0) on success
 * @return -1 on error, @c ptr->_errno set to a constant from errno.h to indicate the error
 */
int _open_r(struct _reent *ptr, const char *name, int flags, int mode)
{
    int fd = vfs_open(name, flags, mode);
    if (fd < 0) {
        /* vfs returns negative error codes */
        ptr->_errno = -fd;
        return -1;
    }
    return fd;
}

/**
 * @brief  Read bytes from an open file
 *
 * This is a wrapper around @c vfs_read
 *
 * @param  ptr    pointer to reent structure
 * @param  fd     open file descriptor obtained from @c open()
 * @param  dest   destination buffer
 * @param  count  maximum number of bytes to read
 *
 * @return number of bytes read on success
 * @return -1 on error, @c r->_errno set to a constant from errno.h to indicate the error
 */
_ssize_t _read_r(struct _reent *ptr, int fd, void *dest, size_t count)
{
    int res = vfs_read(fd, dest, count);
    if (res < 0) {
        /* vfs returns negative error codes */
        ptr->_errno = -res;
        return -1;
    }
    return res;
}

/**
 * @brief  Write bytes to an open file
 *
 * This is a wrapper around @c vfs_write
 *
 * @param  ptr    pointer to reent structure
 * @param  fd     open file descriptor obtained from @c open()
 * @param  src    source data buffer
 * @param  count  maximum number of bytes to write
 *
 * @return number of bytes written on success
 * @return -1 on error, @c ptr->_errno set to a constant from errno.h to indicate the error
 */
_ssize_t _write_r(struct _reent *r, int fd, const void *src, size_t count)
{
    int res = vfs_write(fd, src, count);
    if (res < 0) {
        /* vfs returns negative error codes */
        r->_errno = -res;
        return -1;
    }
    return res;
}

/**
 * @brief  Close an open file
 *
 * This is a wrapper around @c vfs_close
 *
 * If this call returns an error, the fd should still be considered invalid and
 * no further attempt to use it shall be made, not even to retry @c close()
 *
 * @param  ptr    pointer to reent structure
 * @param  fd     open file descriptor obtained from @c open()
 *
 * @return 0 on success
 * @return -1 on error, @c ptr->_errno set to a constant from errno.h to indicate the error
 */
int _close_r(struct _reent *ptr, int fd)
{
    int res = vfs_close(fd);
    if (res < 0) {
        /* vfs returns negative error codes */
        ptr->_errno = -res;
        return -1;
    }
    return res;
}

/**
 * @brief  Query or set options on an open file
 *
 * This is a wrapper around @c vfs_fcntl
 *
 * @param  ptr    pointer to reent structure
 * @param  fd     open file descriptor obtained from @c open()
 * @param  cmd    fcntl command, see man 3p fcntl
 * @param  arg    argument to fcntl command, see man 3p fcntl
 *
 * @return 0 on success
 * @return -1 on error, @c ptr->_errno set to a constant from errno.h to indicate the error
 */
int _fcntl_r (struct _reent *ptr, int fd, int cmd, int arg)
{
    int res = vfs_fcntl(fd, cmd, arg);
    if (res < 0) {
        /* vfs returns negative error codes */
        ptr->_errno = -res;
        return -1;
    }
    return res;
}

/**
 * @brief  Seek to position in file
 *
 * This is a wrapper around @c vfs_lseek
 *
 * @p whence determines the function of the seek and should be set to one of
 * the following values:
 *
 *  - @c SEEK_SET: Seek to absolute offset @p off
 *  - @c SEEK_CUR: Seek to current location + @p off
 *  - @c SEEK_END: Seek to end of file + @p off
 *
 * @param  ptr      pointer to reent structure
 * @param  fd       open file descriptor obtained from @c open()
 * @param  off      seek offset
 * @param  whence   determines the seek method, see detailed description
 *
 * @return the new seek location in the file on success
 * @return -1 on error, @c ptr->_errno set to a constant from errno.h to indicate the error
 */
_off_t _lseek_r(struct _reent *ptr, int fd, _off_t off, int whence)
{
    int res = vfs_lseek(fd, off, whence);
    if (res < 0) {
        /* vfs returns negative error codes */
        ptr->_errno = -res;
        return -1;
    }
    return res;
}

/**
 * @brief  Get status of an open file
 *
 * This is a wrapper around @c vfs_fstat
 *
 * @param  ptr      pointer to reent structure
 * @param  fd       open file descriptor obtained from @c open()
 * @param  buf      pointer to stat struct to fill
 *
 * @return 0 on success
 * @return -1 on error, @c ptr->_errno set to a constant from errno.h to indicate the error
 */
int _fstat_r(struct _reent *ptr, int fd, struct stat *buf)
{
    int res = vfs_fstat(fd, buf);
    if (res < 0) {
        /* vfs returns negative error codes */
        ptr->_errno = -res;
        return -1;
    }
    return 0;
}

/**
 * @brief  Status of a file (by name)
 *
 * This is a wrapper around @c vfs_fstat
 *
 * @param  ptr      pointer to reent structure
 * @param  name     path to file
 * @param  buf      pointer to stat struct to fill
 *
 * @return 0 on success
 * @return -1 on error, @c ptr->_errno set to a constant from errno.h to indicate the error
 */
int _stat_r(struct _reent *ptr, const char *name, struct stat *st)
{
    int res = vfs_stat(name, st);
    if (res < 0) {
        /* vfs returns negative error codes */
        ptr->_errno = -res;
        return -1;
    }
    return 0;
}

/**
 * @brief  Unlink (delete) a file
 *
 * @param  ptr      pointer to reent structure
 * @param  path     path to file to be deleted
 *
 * @return 0 on success
 * @return -1 on error, @c ptr->_errno set to a constant from errno.h to indicate the error
 */
int _unlink_r(struct _reent *ptr, const char *path)
{
    int res = vfs_unlink(path);
    if (res < 0) {
        /* vfs returns negative error codes */
        ptr->_errno = -res;
        return -1;
    }
    return 0;
}

/**
 * @brief Create a hard link (not implemented).
 *
 * Not implemented.
 *
 * @return  -1. Sets errno to ENOSYS.
 */
int _link_r(struct _reent *ptr, const char *old_name, const char *new_name)
{
    (void)old_name;
    (void)new_name;

    ptr->_errno = ENOSYS;

    return -1;
}

/**
 * @brief  Return time information on the current process
 *
 * @param  ptr Pointer to the global data block, which holds errno
 * @param  ptms Pointer to place timing information
 *
 * @return Timing information or a negative value on error.
 */
clock_t _times_r(struct _reent *ptr, struct tms *ptms)
{
    const TickType_t tick_count = xTaskGetTickCount();

    if (NULL != ptms)
    {
        ptms->tms_utime = tick_count;
        ptms->tms_stime = 0;
        ptms->tms_cutime = 0;
        ptms->tms_cstime = 0;
    }

    return tick_count;
}

/**
 * @brief  Gets the time
 *
 * @param  ptr Pointer to the global data block, which holds errno
 * @param  ptimeval
 * @param  ptimezone
 *
 * @return 0 on success, -1 on error and errno is set to indicate the error
 */
int _gettimeofday_r(struct _reent *ptr, struct timeval *ptimeval, void *ptimezone)
{
    (void)ptimezone;

    struct tm t;
    uint16_t ms;
    int res = rtc_get_time_ms(&t, &ms);
    if (res < 0)
    {
        ptr->_errno = EIO;
        return -1;
    }

    if (NULL != ptimeval)
    {
        uint64_t sec = mktime(&t);
        ptimeval->tv_sec = sec - (time_t)(ms / 1000);
        ptimeval->tv_usec = (suseconds_t)(ms * 1000);
    }

    return 0;
}


