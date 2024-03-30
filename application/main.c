#include "FreeRTOS.h"
#include "task.h"

#include "stm32f4xx_hal.h"
#include "rcc.h"

int main(void)
{
    rcc_init();
    HAL_Init();
    rcc_clock_system_init();

    vTaskStartScheduler();

    while (1)
    {

    }
}

