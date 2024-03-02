#include "FreeRTOS.h"
#include "task.h"

#include "stm32f4xx_hal.h"
#include "system_clock.h"

int main(void)
{
    HAL_Init();

    system_clock_init();

    vTaskStartScheduler();

    while (1)
    {

    }
}

