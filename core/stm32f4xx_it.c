/**
 * MIT License
 *
 * Copyright (c) 2024 Balint Kardos
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/**
 * @ingroup     core
 *
 * @file        stm32f4xx_it.c
 * @brief       Cortex-M4 Interrupt and Exception Handlers
 */

#include "stm32f4xx_hal.h"

void _get_registers_from_stack(uint32_t *fault_stack_address)
{
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

    const uint32_t r0 = fault_stack_address[0];
    const uint32_t r1 = fault_stack_address[1];
    const uint32_t r2 = fault_stack_address[2];
    const uint32_t r3 = fault_stack_address[3];

    const uint32_t r12 = fault_stack_address[4];
    const uint32_t lr = fault_stack_address[5];  /* Link register. */
    const uint32_t pc = fault_stack_address[6];  /* Program counter. */
    const uint32_t psr = fault_stack_address[7]; /* Program status register. */

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


