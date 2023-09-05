/*
 * retarget_locks.c
 *
 *  Created on: 2023. aug. 23.
 *      Author: Balint
 */
#include <sys/lock.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

SemaphoreHandle_t h_rtc_mutex;
StaticSemaphore_t rtc_mutex_buffer;

SemaphoreHandle_t h_fs_mutex;
StaticSemaphore_t fs_mutex_buffer;

struct __lock {
    SemaphoreHandle_t   sem;
};

struct __lock __lock___sinit_recursive_mutex;
struct __lock __lock___sfp_recursive_mutex;
struct __lock __lock___atexit_recursive_mutex;
struct __lock __lock___at_quick_exit_mutex;
struct __lock __lock___malloc_recursive_mutex;
struct __lock __lock___env_recursive_mutex;
struct __lock __lock___tz_mutex;
struct __lock __lock___dd_hash_mutex;
struct __lock __lock___arc4random_mutex;


__attribute__((constructor))
static void init_retarget_locks(void)
{
    __lock___sinit_recursive_mutex.sem  = xSemaphoreCreateRecursiveMutex();
    __lock___sfp_recursive_mutex.sem    = xSemaphoreCreateRecursiveMutex();
    __lock___atexit_recursive_mutex.sem = xSemaphoreCreateRecursiveMutex();
    __lock___at_quick_exit_mutex.sem    = xSemaphoreCreateMutex();
    __lock___malloc_recursive_mutex.sem = xSemaphoreCreateRecursiveMutex();
    __lock___env_recursive_mutex.sem    = xSemaphoreCreateRecursiveMutex();
    __lock___tz_mutex.sem               = xSemaphoreCreateMutex();
    __lock___dd_hash_mutex.sem          = xSemaphoreCreateMutex();
    __lock___arc4random_mutex.sem       = xSemaphoreCreateMutex();

    h_rtc_mutex = xSemaphoreCreateMutexStatic(&rtc_mutex_buffer);
    h_fs_mutex = xSemaphoreCreateMutexStatic(&fs_mutex_buffer);
}

void __retarget_lock_init(_LOCK_T *lock_ptr)
{
    *lock_ptr = pvPortMalloc(sizeof(struct __lock));
    (*lock_ptr)->sem = xSemaphoreCreateMutex();
}


void __retarget_lock_init_recursive(_LOCK_T *lock_ptr)
{
    *lock_ptr = pvPortMalloc(sizeof(struct __lock));
    (*lock_ptr)->sem = xSemaphoreCreateRecursiveMutex();
}


void __retarget_lock_close(_LOCK_T lock)
{
    vSemaphoreDelete(lock->sem);
    vPortFree(lock);
}


void __retarget_lock_close_recursive(_LOCK_T lock)
{
    vSemaphoreDelete(lock->sem);
    vPortFree(lock);
}


void __retarget_lock_acquire(_LOCK_T lock)
{
    xSemaphoreTake(lock->sem, portMAX_DELAY);
}


void __retarget_lock_acquire_recursive(_LOCK_T lock)
{
    xSemaphoreTakeRecursive(lock->sem, portMAX_DELAY);
}


int __retarget_lock_try_acquire(_LOCK_T lock)
{
    return xSemaphoreTake(lock->sem, 0);
}


int __retarget_lock_try_acquire_recursive(_LOCK_T lock)
{
    return xSemaphoreTakeRecursive(lock->sem, 0);
}


void __retarget_lock_release(_LOCK_T lock)
{
    xSemaphoreGive(lock->sem);
}


void __retarget_lock_release_recursive(_LOCK_T lock)
{
    xSemaphoreGiveRecursive(lock->sem);
}
