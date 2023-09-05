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
#include <stdint.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>
#include <reent.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "rtc.h"
#include "rtc_utils.h"

typedef struct _FIL
{
    /* data */
} FIL;

#define MAX_FILE_DESCRIPTORS 16
static FIL file_descriptor[MAX_FILE_DESCRIPTORS] = { };

static void rtc_lock(void);
static void rtc_unlock(void);
static void fs_lock(void);
static void fs_unlock(void);
static FIL *get_descr(struct _reent *r, int fd);


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
    taskENTER_CRITICAL();

    TaskHandle_t h_current_task = xTaskGetCurrentTaskHandle();
    int task_id;

    if (NULL == h_current_task)
    {
        ptr->_errno = ESRCH;
        task_id = -1;
    }
    else
    {
        task_id = (int)uxTaskGetTaskNumber(h_current_task);
    }

    taskEXIT_CRITICAL();

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
    taskENTER_CRITICAL();

    TaskStatus_t task_stats;
    vTaskGetInfo(NULL, &task_stats, pdFALSE, eRunning);

    taskEXIT_CRITICAL();

    if (NULL != ptms)
    {
        ptms->tms_utime = task_stats.ulRunTimeCounter;
        ptms->tms_stime = 0;
        ptms->tms_cutime = 0;
        ptms->tms_cstime = 0;
    }

    return task_stats.ulRunTimeCounter;
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

    if (NULL == ptimeval)
    {
        ptr->_errno = EFAULT;
        return -1;
    }

    rtc_lock();

    uint8_t hours, minutes, seconds;
    uint32_t subseconds;
    rtc_get_time(&hours, &minutes, &seconds, &subseconds);

    uint8_t day, month, weekday;
    uint32_t year;
    rtc_get_date(&day, &month, &year, &weekday);

    rtc_unlock();

    struct tm t;
    t.tm_sec = (int)seconds;
    t.tm_min = (int)minutes;
    t.tm_hour = (int)hours;
    t.tm_mday = (int)day;
    t.tm_mon = (int)month;
    t.tm_year = (int)(year - 1900);

    uint64_t ms = (uint64_t)(1000U * rtc_mktime(&t)) + (uint64_t)subseconds;
    ptimeval->tv_sec = (time_t)(ms / 1000U);
    ptimeval->tv_usec = (suseconds_t)(ms % 1000U);

    return 0;
}

/**
 * Pointer to the current high watermark of the heap usage
 */
static uint8_t *__sbrk_heap_end = NULL;

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
    extern uint8_t _end; /* Symbol defined in the linker script */
    extern uint8_t _estack; /* Symbol defined in the linker script */
    extern uint32_t _Min_Stack_Size; /* Symbol defined in the linker script */
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
 * @brief  Locks the RTC mutex
 *
 * @param  None
 *
 * @note   The rtc mutex is initialized in init_retarget_locks()
 */
static void rtc_lock(void)
{
    extern SemaphoreHandle_t h_rtc_mutex;
    xSemaphoreTake(h_rtc_mutex, portMAX_DELAY);
}

/**
 * @brief  Unlocks the RTC mutex
 *
 * @param  None
 *
 * @note   The rtc mutex is initialized in init_retarget_locks()
 */
static void rtc_unlock(void)
{
    extern SemaphoreHandle_t h_rtc_mutex;
    xSemaphoreGive(h_rtc_mutex);
}

/**
 * @brief  Locks the filesystem mutex
 *
 * @param  None
 *
 * @note   The fs mutex is initialized in init_retarget_locks()
 */
static void fs_lock(void)
{
    extern SemaphoreHandle_t h_fs_mutex;
    xSemaphoreTake(h_fs_mutex, portMAX_DELAY);
}

/**
 * @brief  Unlocks the filesystem mutex
 *
 * @param  None
 *
 * @note   The fs mutex is initialized in init_retarget_locks()
 */
static void fs_unlock(void)
{
    extern SemaphoreHandle_t h_fs_mutex;
    xSemaphoreGive(h_fs_mutex);
}

/*
 * @brief Gets the FIL file descriptor based on int fd
 *
 * @param  ptr Pointer to the global data block, which holds errno
 * @param  fd the open file descriptor id
 *
 * @return the corresponding FIL structure based on fd on success,
 *         NULL on error and errno is set to indicate the error
 */
static FIL *get_descr(struct _reent *ptr, int fd)
{
    fd -= 3;
    if (fd < 0 || fd >= MAX_FILE_DESCRIPTORS) {
        ptr->_errno = EBADF;
        return NULL;
    }
    return &file_descriptor[fd];
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

