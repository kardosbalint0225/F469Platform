/**
 ******************************************************************************
 * @file      syscalls.c
 *
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>
#include <reent.h>

#include "FreeRTOS.h"
#include "task.h"

char *__env[1] = { 0 };
char **environ = __env;

void initialise_monitor_handles()
{

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
    int task_id;
    TaskHandle_t h_current_task = xTaskGetCurrentTaskHandle();

    if (NULL == h_current_task)
    {
        ptr->_errno = ESRCH;
        task_id = -1;
    }
    else
    {
        task_id = (int)uxTaskGetTaskNumber(h_current_task);
    }

    return task_id;
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
    while (1)
    {

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
 * @brief  Return time information on the current process
 *
 * @param  ptr Pointer to the global data block, which holds errno
 * @param  ptms Pointer to place timing information
 *
 * @return Timing information or a negative value on error.
 */
clock_t _times_r(struct _reent *ptr, struct tms *ptms)
{
    TaskStatus_t task_stats;
    vTaskGetInfo(NULL, &task_stats, pdFALSE, eRunning );
    ptms->tms_utime = task_stats.ulRunTimeCounter;
    //TODO: ptms other fields
    return task_stats.ulRunTimeCounter;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

//int _getpid(void)
//{
//    return 1;
//}

//int _kill(int pid, int sig)
//{
//    errno = EINVAL;
//    return -1;
//}

//void _exit (int status)
//{
//    _kill(status, -1);
//    while (1)
//    {
//    }                   /* Make sure we hang here */
//}

__attribute__((weak)) int _read(int file, char *ptr, int len)
{
    int DataIdx;

    for (DataIdx = 0; DataIdx < len; DataIdx++)
    {
//        *ptr++ = __io_getchar();
    }

    return len;
}

__attribute__((weak)) int _write(int file, char *ptr, int len)
{
    int DataIdx;

    for (DataIdx = 0; DataIdx < len; DataIdx++)
    {
//        __io_putchar(*ptr++);
    }
    return len;
}

int _close(int file)
{
    return -1;
}


int _fstat(int file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

//int _isatty(int file)
//{
//    return 1;
//}

int _lseek(int file, int ptr, int dir)
{
    return 0;
}

int _open(char *path, int flags, ...)
{
    /* Pretend like we always fail */
    return -1;
}

//int _wait(int *status)
//{
//    errno = ECHILD;
//    return -1;
//}

int _unlink(char *name)
{
    errno = ENOENT;
    return -1;
}

//int _times(struct tms *buf)
//{
//    return -1;
//}

int _stat(char *file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int _link(char *old, char *new)
{
    errno = EMLINK;
    return -1;
}

//int _fork(void)
//{
//    errno = EAGAIN;
//    return -1;
//}

//int _execve(char *name, char **argv, char **env)
//{
//    errno = ENOMEM;
//    return -1;
//}

