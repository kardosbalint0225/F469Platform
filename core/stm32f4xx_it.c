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

extern UART_HandleTypeDef h_stdio_uart;
extern DMA_HandleTypeDef h_stdio_uart_dma_tx;
extern TIM_HandleTypeDef h_tim2;
extern TIM_HandleTypeDef h_tim6;
extern EXTI_HandleTypeDef h_exti_sdcard_cd_pin;
extern SD_HandleTypeDef h_sdio;
extern DMA_HandleTypeDef h_sdio_dma_tx;
extern DMA_HandleTypeDef h_sdio_dma_rx;
extern RTC_HandleTypeDef hrtc;

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
 * @brief This function handles DMA1 stream3 global interrupt.
 */
void DMA1_Stream3_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&h_stdio_uart_dma_tx);
}

/**
 * @brief This function handles USART3 global interrupt.
 */
void USART3_IRQHandler(void)
{
    HAL_UART_IRQHandler(&h_stdio_uart);
}

/**
 * @brief This function handles TIM2 global interrupt
 */
void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&h_tim2);
}

/**
 * @brief This function handles TIM6 global interrupt, DAC1 and DAC2 underrun error interrupts.
 */
void TIM6_DAC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&h_tim6);
}

/**
 * @brief This function handles EXTI line 2 global interrupt
 */
void EXTI2_IRQHandler(void)
{
    HAL_EXTI_IRQHandler(&h_exti_sdcard_cd_pin);
}

/**
 * @brief This function handles DMA2 stream3 global interrupt.
 */
void DMA2_Stream3_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&h_sdio_dma_rx);
}

/**
 * @brief This function handles DMA2 stream6 global interrupt.
 */
void DMA2_Stream6_IRQHandler(void)
{
    HAL_DMA_IRQHandler(&h_sdio_dma_tx);
}

/**
 * @brief This function handles SDIO global interrupt.
 */
void SDIO_IRQHandler(void)
{
    HAL_SD_IRQHandler(&h_sdio);
}

/**
 * @brief This function handles RTC WakeUp global interrupt.
 */
void RTC_WKUP_IRQHandler(void)
{
    HAL_RTCEx_WakeUpTimerIRQHandler(&hrtc);
}

