#ifndef __FREERTOS_TASKS_C_ADDITIONS_H__
#define __FREERTOS_TASKS_C_ADDITIONS_H__

#ifdef __cplusplus
extern "C" {
#endif

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

