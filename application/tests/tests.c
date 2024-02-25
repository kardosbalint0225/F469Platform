#if RUN_TESTS

#include "tests.h"
#include "tests_config.h"

#include "FreeRTOS.h"
#include "task.h"

#include <limits.h>
#include <string.h>
#include <stdio.h>

#include "stdio_base.h"

static void tests_runner_task(void *params);
static void stdin1_test_task(void *params);
static void stdin2_test_task(void *params);
static void stdin3_test_task(void *params);
static void stdin4_test_task(void *params);
static void stdio_uart_init_deinit_test_task(void *params);

void start_tests(void)
{
    xTaskCreate(tests_runner_task,
                "Tests",
                configMINIMAL_STACK_SIZE * 10,
                NULL,
                (tskIDLE_PRIORITY + 1),
                NULL);
}

static void tests_runner_task(void * params)
{
    xTaskCreate(stdin1_test_task,
                "stdin1test",
                configMINIMAL_STACK_SIZE * 10,
                NULL,
                (tskIDLE_PRIORITY + 1),
                NULL);
    xTaskCreate(stdin2_test_task,
                "stdin2test",
                configMINIMAL_STACK_SIZE * 10,
                NULL,
                (tskIDLE_PRIORITY + 1),
                NULL);
    xTaskCreate(stdin3_test_task,
                "stdin3test",
                configMINIMAL_STACK_SIZE * 10,
                NULL,
                (tskIDLE_PRIORITY + 2),
                NULL);
    xTaskCreate(stdin4_test_task,
                "stdin4test",
                configMINIMAL_STACK_SIZE * 10,
                NULL,
                (tskIDLE_PRIORITY + 3),
                NULL);
    xTaskCreate(stdio_uart_init_deinit_test_task,
                "stdin4test",
                configMINIMAL_STACK_SIZE * 10,
                NULL,
                (tskIDLE_PRIORITY + 3),
                NULL);
    for ( ;; )
    {
        vTaskDelay(500);
    }
}

static void stdio_uart_init_deinit_test_task(void *params)
{
    for ( ;; )
    {
//        stdio_deinit();
//        vTaskDelay(1);
//        stdio_init();
//        printf("hello.\r\n");
//        vTaskDelay(1);
//
        vTaskDelay(500);
    }
}

static void stdin1_test_task(void *params)
{
    for ( ;; )
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        char str[33];
        scanf("%32s", str);
        printf("%s\r\n", str);
    }
}

static void stdin2_test_task(void *params)
{
    for ( ;; )
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        char str[33];
        scanf("%32s", str);
        printf("%s\r\n", str);
    }
}

static void stdin3_test_task(void *params)
{
    for ( ;; )
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        char str[33];
        scanf("%32s", str);
        printf("%s\r\n", str);
    }
}

static void stdin4_test_task(void *params)
{
    for ( ;; )
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        char str[33];
        scanf("%32s", str);
        printf("%s\r\n", str);
    }
}

#endif

