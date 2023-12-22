

#include "FreeRTOS.h"
#include "task.h"

#include "stm32f4xx_hal.h"
#include "sysclk.h"

int main(void)
{
    HAL_Init();
    sysclk_init();

    vTaskStartScheduler();

    while (1)
    {

    }
}

