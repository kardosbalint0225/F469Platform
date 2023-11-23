/*
 * freertos_tasks_c_additions.h
 *
 *  Created on: 2023. nov. 19.
 *      Author: Balint
 */

#ifndef _FREERTOS_TASKS_C_ADDITIONS_H_
#define _FREERTOS_TASKS_C_ADDITIONS_H_

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

#endif /* _FREERTOS_TASKS_C_ADDITIONS_H_ */
