#include "stm32f4xx_it.h"
#include "stm32f4xx_hal.h"

extern UART_HandleTypeDef h_stdio_uart;
extern DMA_HandleTypeDef h_stdio_uart_dma_tx;
extern TIM_HandleTypeDef h_runtime_stats_timer;
extern TIM_HandleTypeDef h_hal_timebase_tim;
extern EXTI_HandleTypeDef h_exti_sdcard_cd_pin;
extern SD_HandleTypeDef h_sdio;
extern DMA_HandleTypeDef h_sdio_dma_tx;
extern DMA_HandleTypeDef h_sdio_dma_rx;
extern RTC_HandleTypeDef h_rtc;
extern HCD_HandleTypeDef h_hcd_fs;
extern EXTI_HandleTypeDef h_exti_usb_host_overcurrent_pin;

void _get_registers_from_stack(uint32_t *fault_stack_address)
{
    const uint32_t r0 = fault_stack_address[0];
    const uint32_t r1 = fault_stack_address[1];
    const uint32_t r2 = fault_stack_address[2];
    const uint32_t r3 = fault_stack_address[3];

    const uint32_t r12 = fault_stack_address[4];
    const uint32_t lr = fault_stack_address[5];  /* Link register. */
    const uint32_t pc = fault_stack_address[6];  /* Program counter. */
    const uint32_t psr = fault_stack_address[7]; /* Program status register. */

    const uint32_t BFARVALID_MASK = (0x80 << SCB_CFSR_BUSFAULTSR_Pos);
    const uint32_t MMARVALID_MASK = (0x80 << SCB_CFSR_MEMFAULTSR_Pos);
    const uint32_t bfar = SCB->BFAR;
    const uint32_t mmfar = SCB->MMFAR;
    const uint32_t cfsr = SCB->CFSR;
    const uint32_t hfsr = SCB->HFSR;
    const uint32_t dfsr = SCB->DFSR;
    const uint32_t afsr = SCB->AFSR;
    const uint32_t bfar_valid = cfsr & BFARVALID_MASK;
    const uint32_t mmfar_valid = cfsr & MMARVALID_MASK;

    (void)bfar;
    (void)mmfar;
    (void)cfsr;
    (void)hfsr;
    (void)dfsr;
    (void)afsr;
    (void)bfar_valid;
    (void)mmfar_valid;

    (void)r0;
    (void)r1;
    (void)r2;
    (void)r3;

    (void)r12;
    (void)lr;
    (void)pc;
    (void)psr;

    /* When the following line is hit, the variables contain the register values. */
    for( ;; );
}

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
__attribute__ ((naked, aligned(8))) void HardFault_Handler(void)
{
    __asm volatile
    (
        " tst lr, #4                                                \n"
        " ite eq                                                    \n"
        " mrseq r0, msp                                             \n"
        " mrsne r0, psp                                             \n"
        " ldr r1, [r0, #24]                                         \n"
        " ldr r2, handler2_address_const                            \n"
        " bx r2                                                     \n"
        " handler2_address_const: .word _get_registers_from_stack   \n"
    );
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

    while (1)
    {

    }
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
    HAL_TIM_IRQHandler(&h_runtime_stats_timer);
}

/**
 * @brief This function handles TIM6 global interrupt, DAC1 and DAC2 underrun error interrupts.
 */
void TIM6_DAC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&h_hal_timebase_tim);
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
    HAL_RTCEx_WakeUpTimerIRQHandler(&h_rtc);
}

/**
 * @brief This function handles USB FS global interrupt.
 */
void OTG_FS_IRQHandler(void)
{
    HAL_HCD_IRQHandler(&h_hcd_fs);
}

/**
 * @brief This function handles External Line[9:5] global interrupts .
 */
void EXTI9_5_IRQHandler(void)
{
    HAL_EXTI_IRQHandler(&h_exti_usb_host_overcurrent_pin);
}

