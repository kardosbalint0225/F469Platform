/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  *
  *
  ******************************************************************************
  */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

#include "stm32f4xx_hal.h"

void SystemClock_Config(void);
void PeriphCommonClock_Config(void);

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  HAL_Init();
  SystemClock_Config();
  PeriphCommonClock_Config();

  vTaskStartScheduler();

  while (1)
  {

  }
}

