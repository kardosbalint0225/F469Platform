#if RUN_TESTS

#include "tests.h"
#include "tests_config.h"

#include "FreeRTOS.h"
#include "task.h"

#include <limits.h>
#include <string.h>
#include <stdio.h>

#include "stdio_base.h"
#include "gpio.h"

static void tests_runner_task(void *params);

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
    led1_pin_init();

    for ( ;; )
    {
        vTaskDelay(500);
    }
}

#endif

