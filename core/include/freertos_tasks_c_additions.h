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
 * @ingroup     core_util
 *
 * @file        freertos_tasks_c_additions.h
 * @brief       Extension method to FreeRTOS
 */
#ifndef __FREERTOS_TASKS_C_ADDITIONS_H__
#define __FREERTOS_TASKS_C_ADDITIONS_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief     Gets the stack size of the given task.
 *
 * This function retrieves the stack size of the task identified by the provided
 * task handle. It calculates the stack size based on the end address and start
 * address of the stack.
 *
 * @param[in] xTask Task handle of the task whose stack size is to be obtained.
 *
 * @return    The size of the stack (in bytes) of the specified task.
 * @return    If the task handle is NULL, returns 0.
 */
uint32_t uxTaskGetStackSize( TaskHandle_t xTask )
{
    uint32_t uxReturn;
    TCB_t *pxTCB;

    if ( NULL != xTask )
    {
        pxTCB = ( TCB_t * ) xTask;
        uxReturn = ( pxTCB->pxEndOfStack - pxTCB->pxStack ) * sizeof(StackType_t);
    }
    else
    {
        uxReturn = 0;
    }

    return uxReturn;
}

#ifdef __cplusplus
}
#endif
#endif /* __FREERTOS_TASKS_C_ADDITIONS_H__ */

