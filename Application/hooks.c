#include "FreeRTOS.h"
#include "task.h"

#include "stm32f4xx_hal.h"
#include "gpio.h"
#include "rtc.h"
#include "stdio_base.h"
#include "cli.h"
#include "vfs.h"
#include "sdcard_mount.h"
#include "usb_host.h"
#include "cwd.h"
#include "panic.h"
#include <stdio.h>

#if RUN_TESTS
    #include "tests.h"
#endif

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize);

/* GetTimerTaskMemory prototype (linked to static allocation support) */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize);

/* Hook prototypes */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName);
void vApplicationMallocFailedHook(void);
void vApplicationDaemonTaskStartupHook(void);

void vApplicationDaemonTaskStartupHook(void)
{
    rtc_init();
    vfs_init();
    stdio_init();
    vfs_bind_stdio();
    setvbuf(stdin, NULL, _IONBF, 0);
    sdcard_mount_init();
    usb_host_init();
    cli_init();
    cwd_init();
#if RUN_TESTS
    start_tests();
#endif
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    panic("\r\n Task stack overflowed. Task: %s\r\n", pcTaskName);
}

void vApplicationMallocFailedHook(void)
{
    panic("\r\n Not enough memory in system heap. (malloc failed hook checked)\r\n");
}

/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
 * implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
 * used by the Idle task.
 * */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = &uxIdleTaskStack[0];
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/* configSUPPORT_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 * application must provide an implementation of vApplicationGetTimerTaskMemory()
 * to provide the memory that is used by the Timer service task.
 * */
static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize)
{
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = &uxTimerTaskStack[0];
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

