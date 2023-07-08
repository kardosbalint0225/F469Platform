/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  *
  *
  ******************************************************************************
  */
#include "stm32f4xx_it.h"
#include "stm32f4xx_hal.h"

//extern HCD_HandleTypeDef hhcd_USB_OTG_FS;
//extern DMA2D_HandleTypeDef hdma2d;
//extern DSI_HandleTypeDef hdsi;
//extern LTDC_HandleTypeDef hltdc;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim6;

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{

  while (1)
  {

  }
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{

  while (1)
  {

  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{

  while (1)
  {

  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{

  while (1)
  {

  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{

  while (1)
  {

  }
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{

}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles TIM2 global interrupt
  */
void TIM2_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim2);
}

/**
  * @brief This function handles TIM6 global interrupt, DAC1 and DAC2 underrun error interrupts.
  */
void TIM6_DAC_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim6);
}

///**
//  * @brief This function handles USB On The Go FS global interrupt.
//  */
//void OTG_FS_IRQHandler(void)
//{
//  HAL_HCD_IRQHandler(&hhcd_USB_OTG_FS);
//}

///**
//  * @brief This function handles LTDC global interrupt.
//  */
//void LTDC_IRQHandler(void)
//{
//  HAL_LTDC_IRQHandler(&hltdc);
//}

///**
//  * @brief This function handles DMA2D global interrupt.
//  */
//void DMA2D_IRQHandler(void)
//{
//  HAL_DMA2D_IRQHandler(&hdma2d);
//}

///**
//  * @brief This function handles DSI global interrupt.
//  */
//void DSI_IRQHandler(void)
//{
//  HAL_DSI_IRQHandler(&hdsi);
//}

