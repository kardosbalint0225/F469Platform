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

#include "uart_console.h"
#include <string.h>

static char *test_strings[6] = {
		"This is a text message.\r\n",
		"Thats another text message.\r\n",
		"I am not very creative when it comes to writing test text messages.\r\n",
		"Lorem Ipsum is simply dummy text of the printing and typesetting industry.\r\n",
		"Contrary to popular belief, Lorem Ipsum is not simply random text.\r\n",
		"It is a long established fact that a reader will be distracted by the readable content of a page when looking at its layout.\r\n",
};

static void tests_runner_task(void * params);

void start_tests(void)
{
	/** Init tests runner task */
    xTaskCreate( tests_runner_task,
                 "Tests",
                 configMINIMAL_STACK_SIZE*64,
                 NULL,
				 ( tskIDLE_PRIORITY + 1 ),
                 NULL );
}


static void tests_runner_task(void * params)
{
	for ( ;; )
	{
		int size = 6;
		for (int i = 0; i < size; i++)
		{
			char *pbuf = test_strings[i];
			int len = strlen(pbuf);
			__uart_console_write(pbuf, len);
		}
		vTaskDelay(500);
	}
}

#endif
