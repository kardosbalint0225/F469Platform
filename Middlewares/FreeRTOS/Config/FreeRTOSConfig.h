#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * These parameters and more are described within the 'configuration' section of the
 * FreeRTOS API documentation available on the FreeRTOS.org web site.
 *
 * See http://www.freertos.org/a00110.html
 *----------------------------------------------------------*/

#include <stdint.h>
#include "stm32f4xx_hal.h"

extern uint32_t SystemCoreClock;

#define configENABLE_FPU                              ( 0 ) // default 1 (FreeRTOS.h) ARMv8M
#define configENABLE_MPU                              ( 0 ) // default 0 (FreeRTOS.h) ARMv8M
#define configUSE_PREEMPTION                          ( 1 )                 
#define configUSE_PORT_OPTIMISED_TASK_SELECTION       ( 1 )                 
#define configUSE_TICKLESS_IDLE                       ( 0 ) // default 0 (FreeRTOS.h)                
#define configCPU_CLOCK_HZ                            ( SystemCoreClock )   
//#define configSYSTICK_CLOCK_HZ                        configCPU_CLOCK_HZ
#define configTICK_RATE_HZ                            ( (TickType_t)1000 )  
//#define configINITIAL_TICK_COUNT                          // default 0 (FreeRTOS.h)
#define configMAX_PRIORITIES                          ( 7 )                 
#define configMINIMAL_STACK_SIZE                      ( (uint16_t)128 )
//#define configMINIMAL_SECURE_STACK_SIZE                   // ? (used in tasks.c)
#define configMAX_TASK_NAME_LEN                       ( 16 )// default 16 (FreeRTOS.h)
#define configUSE_16_BIT_TICKS                        ( 0 )
#define configIDLE_SHOULD_YIELD                       ( 1 ) // default 1 (FreeRTOS.h) 
//#define configIDLE_TASK_NAME                              // default "IDLE" (task.c)
#define configUSE_TASK_NOTIFICATIONS                  ( 1 ) // default 1 (FreeRTOS.h)
#define configTASK_NOTIFICATION_ARRAY_ENTRIES         ( 1 ) // default 1 (FreeRTOS.h)
#define configUSE_MUTEXES                             ( 1 ) // default 0 (FreeRTOS.h)
#define configUSE_RECURSIVE_MUTEXES                   ( 1 ) // default 0 (FreeRTOS.h)
#define configUSE_COUNTING_SEMAPHORES                 ( 1 ) // default 0 (FreeRTOS.h)
#define configUSE_ALTERNATIVE_API                     ( 0 ) // default 0 (FreeRTOS.h)
#define configQUEUE_REGISTRY_SIZE                     ( 8 ) // default 0 (FreeRTOS.h)
#define configUSE_QUEUE_SETS                          ( 0 )	// default 0 (FreeRTOS.h)
#define configUSE_TIME_SLICING                        ( 1 ) // default 1 (FreeRTOS.h)
#define configUSE_NEWLIB_REENTRANT                    ( 0 ) // default 0 (FreeRTOS.h)
#define configENABLE_BACKWARD_COMPATIBILITY           ( 0 ) // default 1 (FreeRTOS.h)
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS       ( 5 ) // default 0 (FreeRTOS.h)
#define configSTACK_DEPTH_TYPE                        uint16_t // default uint16_t (FreeRTOS.h)
//#define configRECORD_STACK_HIGH_ADDRESS                   // default 0 (FreeRTOS.h)
#define configMESSAGE_BUFFER_LENGTH_TYPE              size_t // default size_t (FreeRTOS.h)
//#define configUSE_LIST_DATA_INTEGRITY_CHECK_BYTES         // default 0 (projdefs.h)
//#define configEXPECTED_IDLE_TIME_BEFORE_SLEEP             // default 2 (FreeRTOS.h)
//#define configPRE_SUPPRESS_TICKS_AND_SLEEP_PROCESSING     // default empty (FreeRTOS.h)
//#define configLIST_VOLATILE                               // default empty (list.h)
//#define configINCLUDE_FREERTOS_TASK_C_ADDITIONS_H         // default 0 (FreeRTOS.h)
//#define configPRE_SLEEP_PROCESSING                        // default empty (FreeRTOS.h)
//#define configPOST_SLEEP_PROCESSING                       // default empty (FreeRTOS.h)
//#define configUSE_TASK_FPU_SUPPORT                        // default 1 (FreeRTOS.h)
//#define configENABLE_TRUSTZONE                            // default 1 (FreeRTOS.h) ARMv8M
//#define configRUN_FREERTOS_SECURE_ONLY                    // default 0 (FreeRTOS.h)
//#define configRUN_ADDITIONAL_TESTS                        // default 0 (FreeRTOS.h)
//#define configSUPPORT_CROSS_MODULE_OPTIMISATION           // ? (list.h)
//#define configTASK_RETURN_ADDRESS                         // default prvTaskExitError (port.c)

/* Memory allocation related definitions. */
#define configSUPPORT_STATIC_ALLOCATION               ( 1 ) // default 0 (FreeRTOS.h)
#define configSUPPORT_DYNAMIC_ALLOCATION              ( 1 ) // default 1 (FreeRTOS.h)
#define configTOTAL_HEAP_SIZE                         ( (size_t)0x10000 )
#define configAPPLICATION_ALLOCATED_HEAP              ( 1 ) // default 0 (FreeRTOS.h)
#define configSTACK_ALLOCATION_FROM_SEPARATE_HEAP     ( 0 ) // default 0 (FreeRTOS.h)
//#define configADJUSTED_HEAP_SIZE                          // heap_1 and heap_2 defines it accordingly

/* Hook function related definitions. */
#define configUSE_IDLE_HOOK                           ( 0 )
#define configUSE_TICK_HOOK                           ( 0 )
#define configCHECK_FOR_STACK_OVERFLOW                ( 2 ) // default 0 (FreeRTOS.h)
#define configUSE_MALLOC_FAILED_HOOK                  ( 1 ) // default 0 (FreeRTOS.h)
#define configUSE_DAEMON_TASK_STARTUP_HOOK            ( 1 ) // default 0 (FreeRTOS.h)

/* Run time and task stats gathering related definitions. */
#define configGENERATE_RUN_TIME_STATS                 ( 1 ) // default 0 (FreeRTOS.h)
#define configUSE_TRACE_FACILITY                      ( 1 ) // default 0 (FreeRTOS.h)
#define configUSE_STATS_FORMATTING_FUNCTIONS          ( 1 ) // default 0 (FreeRTOS.h)
//#define configRUN_TIME_COUNTER_TYPE                       // default uint32_t (FreeRTOS.h)

/* Co-routine related definitions. */
#define configUSE_CO_ROUTINES                         ( 0 ) // default 0 (FreeRTOS.h)
#define configMAX_CO_ROUTINE_PRIORITIES               ( 1 )

/* Software timer related definitions. */
#define configUSE_TIMERS                              ( 1 ) // default 0 (FreeRTOS.h)
#define configTIMER_TASK_PRIORITY                     ( 3 )
#define configTIMER_QUEUE_LENGTH                      ( 10 )
#define configTIMER_TASK_STACK_DEPTH                  ( configMINIMAL_STACK_SIZE * 2)
//#define configTIMER_SERVICE_TASK_NAME                     // default "Tmr Svc" (timers.c)

/* FreeRTOS POSIX dependencies */
#define configUSE_POSIX_ERRNO                         ( 1 ) // default 0 (FreeRTOS.h)
#define configUSE_APPLICATION_TASK_TAG                ( 1 ) // default 0 (FreeRTOS.h)

/* Interrupt nesting behaviour configuration. */

/* Cortex-M specific definitions. */
#ifdef __NVIC_PRIO_BITS
    /* __NVIC_PRIO_BITS will be specified when CMSIS is being used. */
    #define configPRIO_BITS                           __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS                           ( 4 )
#endif

/* The lowest interrupt priority that can be used in a call to a "set priority"
 * function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY       ( 15 )

/* The highest interrupt priority that can be used by any interrupt service
 * routine that makes calls to interrupt safe FreeRTOS API functions.  
 * DO NOT CALL INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
 * PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY  ( 5 )

/* Interrupt priorities used by the kernel port layer itself.  These are generic
 * to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY 		      ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY      << (8 - configPRIO_BITS) )

/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
 * See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	      ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/* configMAX_API_CALL_INTERRUPT_PRIORITY is a new name for configMAX_SYSCALL_INTERRUPT_PRIORITY 
 * that is used by newer ports only. The two are equivalent. */
//#define configMAX_API_CALL_INTERRUPT_PRIORITY         [dependent on processor and application]

/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
 * standard names. */
#define vPortSVCHandler                               SVC_Handler
#define xPortPendSVHandler                            PendSV_Handler
#define xPortSysTickHandler                           SysTick_Handler

/* Define to trap errors during development. */
//#define configASSERT_DEFINED                        // FreeRTOS.h sets it according to configASSERT
#define configASSERT( x )                         	  assert_param( ( x ) ) // default empty (FreeRTOS.h)

//#define configMIN                                   // FreeRTOS.h provides this MIN macro
//#define configMAX                                   // FreeRTOS.h provides this MAX macro
//#define configPRECONDITION                          // default configASSERT (FreeRTOS.h)
//#define configPRECONDITION_DEFINED                  // default 0
//#define configPRINTF                                // default empty (FreeRTOS.h)

/* FreeRTOS MPU specific definitions. */
#define configINCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS ( 0 ) // default 0 (FreeRTOS.h)

/* Optional functions - most linkers will remove unused functions anyway. */
#define INCLUDE_vTaskPrioritySet                      ( 1 ) // default 0 (FreeRTOS.h)
#define INCLUDE_uxTaskPriorityGet                     ( 1 ) // default 0 (FreeRTOS.h)
#define INCLUDE_vTaskDelete                           ( 1 ) // default 0 (FreeRTOS.h)
#define INCLUDE_vTaskSuspend                          ( 1 ) // default 0 (FreeRTOS.h)
//#define INCLUDE_xResumeFromISR                      ( 1 ) // ?
//#define INCLUDE_vTaskDelayUntil                     ( 1 ) // deprecated use INCLUDE_xTaskDelayUntil instead
#define INCLUDE_xTaskDelayUntil                       ( 1 ) // default 0 (FreeRTOS.h)
#define INCLUDE_vTaskDelay                            ( 1 ) // default 0 (FreeRTOS.h)
#define INCLUDE_xTaskGetSchedulerState                ( 1 ) // default 0 (FreeRTOS.h)
#define INCLUDE_xTaskGetCurrentTaskHandle             ( 1 ) // default 0 (FreeRTOS.h)
#define INCLUDE_uxTaskGetStackHighWaterMark           ( 1 ) // default 0 (FreeRTOS.h)
#define INCLUDE_uxTaskGetStackHighWaterMark2          ( 1 ) // default 0 (FreeRTOS.h)
#define INCLUDE_xTaskGetIdleTaskHandle                ( 1 ) // default 0 (FreeRTOS.h)
#define INCLUDE_eTaskGetState                         ( 1 ) // default 0 (FreeRTOS.h)
//#define INCLUDE_xEventGroupSetBitFromISR            ( 1 ) // ?
#define INCLUDE_xTimerPendFunctionCall                ( 1 ) // default 0 (FreeRTOS.h)
#define INCLUDE_xTaskAbortDelay                       ( 1 ) // default 0 (FreeRTOS.h)
#define INCLUDE_xTaskGetHandle                        ( 1 ) // default 0 (FreeRTOS.h)
#define INCLUDE_xTaskResumeFromISR                    ( 1 ) // default 1 (FreeRTOS.h)
//#define INCLUDE_vTaskCleanUpResources               ( 1 ) // ?
//#define INCLUDE_xSemaphoreGetMutexHolder            ( 1 ) // default INCLUDE_xQueueGetMutexHolder
#define INCLUDE_xQueueGetMutexHolder                  ( 1 ) // default 0 (FreeRTOS.h)


/*********************************************************************************************************/
/**< Run-time statistics */
extern void run_time_stats_timer_init( void );
extern volatile uint32_t runtime_stats_timer;

#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() 	  run_time_stats_timer_init()
#define portGET_RUN_TIME_COUNTER_VALUE()         	  ( runtime_stats_timer )
/*********************************************************************************************************/

#endif /* FREERTOS_CONFIG_H */




