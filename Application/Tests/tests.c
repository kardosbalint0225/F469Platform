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

//static char *test_strings[6] = {
//    "This is a text message.\r\n",
//    "Thats another text message.\r\n",
//    "I am not very creative when it comes to writing test text messages.\r\n",
//    "Lorem Ipsum is simply dummy text of the printing and typesetting industry.\r\n",
//    "Contrary to popular belief, Lorem Ipsum is not simply random text.\r\n",
//    "It is a long established fact that a reader will be distracted by the readable content of a page when looking at its layout.\r\n",
//};

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
    for ( ;; )
    {
//        for (int i = 0; i < 10; i++)
//        {
//            int size = 6;
//            for (int i = 0; i < size; i++)
//            {
//                char *pbuf = test_strings[i];
//                int len = strlen(pbuf);
//                stdio_write(pbuf, len);
//            }
//            vTaskDelay(500);
//        }
//        assert_param(0);
        extern uint8_t _end; /* Symbol defined in the linker script */
        extern uint8_t _estack; /* Symbol defined in the linker script */
        extern uint32_t _Min_Stack_Size; /* Symbol defined in the linker script */
        const uint32_t stack_limit = (uint32_t)&_estack - (uint32_t)&_Min_Stack_Size;
        const size_t max_heap_size = (stack_limit - (uint32_t)&_end) >> 1;
        uint8_t *pbuf = (uint8_t *)malloc(max_heap_size);
        assert_param(NULL != pbuf);
        free(pbuf);
        tzset();

        char *env = NULL;
        setenv("ENVVAR1", "1", 1);
        env = getenv("ENVVAR1");
//        stdio_write(env, strlen(env));
        printf("ENVVAR1: %s\r\n", env);

        setenv("ENVVAR2", "2", 1);
        env = getenv("ENVVAR2");
//        stdio_write(env, strlen(env));
        printf("ENVVAR2: %s\r\n", env);

        setenv("ENVVAR3", "3", 1);
        env = getenv("ENVVAR3");
//        stdio_write(env, strlen(env));
        printf("ENVVAR3: %s\r\n", env);

        setenv("ENVVAR4", "4", 1);
        env = getenv("ENVVAR4");
//        stdio_write(env, strlen(env));
        printf("ENVVAR4: %s\r\n", env);

        setenv("ENVVAR5", "5", 1);
        env = getenv("ENVVAR5");
//        stdio_write(env, strlen(env));
        printf("ENVVAR5: %s\r\n", env);

        env = getenv("ENVVAR_NONEXISTING");
        if (NULL != env)
//            stdio_write(env, strlen(env));
            printf("ENVVAR_NONEXISTING: %s\r\n", env);

        vTaskDelay(500);
    }
}

#endif

