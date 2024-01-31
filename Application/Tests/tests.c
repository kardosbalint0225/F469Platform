#if RUN_TESTS

#include "tests.h"
#include "tests_config.h"

#include "FreeRTOS.h"
#include "task.h"

#include <string.h>
#include <stdio_base.h>

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
    const char *imalive = "Im alive!\r\n";
    const size_t len = strnlen(imalive, 128);
    for ( ;; )
    {
        stdio_write((const void *)imalive, (size_t)len);
        vTaskDelay(500);
    }
}

#endif

