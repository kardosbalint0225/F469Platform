/*
 * tests.c
 *
 *  Created on: 2023. apr. 5.
 *      Author: Balint
 */
#if RUN_TESTS

#include "tests.h"
#include "tests_config.h"

#include "FreeRTOS.h"
#include "task.h"

#include "stdio_base.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>

extern uint32_t uxTaskGetStackSize( TaskHandle_t xTask );

static void tests_runner_task(void * params);

void start_tests(void)
{
    /** Init tests runner task */
    xTaskCreate(tests_runner_task,
                "Tests",
                configMINIMAL_STACK_SIZE * 64,
                NULL,
                (tskIDLE_PRIORITY + 1),
                NULL);
}


static void tests_runner_task(void * params)
{
    TaskHandle_t current = xTaskGetCurrentTaskHandle();
    uint32_t stack_size = uxTaskGetStackSize(current);

    printf("Stack size: %lu\n", stack_size);

    for ( ;; )
    {
        vTaskDelay(10);
    }
}

#endif

