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

#define configUSE_PREEMPTION                          ( 1 )                 
#define configUSE_PORT_OPTIMISED_TASK_SELECTION       ( 1 )                 
#define configUSE_TICKLESS_IDLE                       ( 0 )
#define configCPU_CLOCK_HZ                            ( SystemCoreClock )   
#define configTICK_RATE_HZ                            ( (TickType_t)1000 )  
#define configMAX_PRIORITIES                          ( 7 )                 
#define configMINIMAL_STACK_SIZE                      ( (uint16_t)128 )
#define configMAX_TASK_NAME_LEN                       ( 16 )
#define configUSE_16_BIT_TICKS                        ( 0 )
#define configIDLE_SHOULD_YIELD                       ( 1 )
#define configUSE_TASK_NOTIFICATIONS                  ( 1 )
#define configTASK_NOTIFICATION_ARRAY_ENTRIES         ( 1 )
#define configUSE_MUTEXES                             ( 1 )
#define configUSE_RECURSIVE_MUTEXES                   ( 1 )
#define configUSE_COUNTING_SEMAPHORES                 ( 1 )
#define configUSE_ALTERNATIVE_API                     ( 0 )
#define configQUEUE_REGISTRY_SIZE                     ( 8 )
#define configUSE_QUEUE_SETS                          ( 0 )
#define configUSE_TIME_SLICING                        ( 1 )
#define configUSE_NEWLIB_REENTRANT                    ( 1 )
#define configENABLE_BACKWARD_COMPATIBILITY           ( 0 )
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS       ( 5 )
#define configSTACK_DEPTH_TYPE                        uint16_t
#define configMESSAGE_BUFFER_LENGTH_TYPE              size_t

/* Memory allocation related definitions. */
#define configSUPPORT_STATIC_ALLOCATION               ( 1 )
#define configSUPPORT_DYNAMIC_ALLOCATION              ( 1 )
#define configTOTAL_HEAP_SIZE                         ( (size_t)0x10000 )
#define configAPPLICATION_ALLOCATED_HEAP              ( 0 )
#define configSTACK_ALLOCATION_FROM_SEPARATE_HEAP     ( 0 )

/* Hook function related definitions. */
#define configUSE_IDLE_HOOK                           ( 0 )
#define configUSE_TICK_HOOK                           ( 0 )
#define configCHECK_FOR_STACK_OVERFLOW                ( 2 )
#define configUSE_MALLOC_FAILED_HOOK                  ( 1 )
#define configUSE_DAEMON_TASK_STARTUP_HOOK            ( 1 )

/* Run time and task stats gathering related definitions. */
#define configGENERATE_RUN_TIME_STATS                 ( 1 )
#define configUSE_TRACE_FACILITY                      ( 1 )
#define configUSE_STATS_FORMATTING_FUNCTIONS          ( 1 )

#include "runtime_stats_timer.h"
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() 	  runtime_stats_timer_init()
#define portGET_RUN_TIME_COUNTER_VALUE()         	  runtime_stats_timer_get_count()

/* Co-routine related definitions. */
#define configUSE_CO_ROUTINES                         ( 0 )
#define configMAX_CO_ROUTINE_PRIORITIES               ( 1 )

/* Software timer related definitions. */
#define configUSE_TIMERS                              ( 1 )
#define configTIMER_TASK_PRIORITY                     ( 3 )
#define configTIMER_QUEUE_LENGTH                      ( 10 )
#define configTIMER_TASK_STACK_DEPTH                  ( configMINIMAL_STACK_SIZE * 2)

/* Interrupt nesting behaviour configuration. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY       ( 15 )
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY  ( 5 )
#define configKERNEL_INTERRUPT_PRIORITY 		      ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY      << (8 - __NVIC_PRIO_BITS) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 	      ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - __NVIC_PRIO_BITS) )

/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
 * standard names. */
#define vPortSVCHandler                               SVC_Handler
#define xPortPendSVHandler                            PendSV_Handler
#define xPortSysTickHandler                           SysTick_Handler

/* Define to trap errors during development. */
#define configASSERT( x )                         	  assert_param( ( x ) )

/* Optional functions - most linkers will remove unused functions anyway. */
#define INCLUDE_vTaskPrioritySet                      ( 1 )
#define INCLUDE_uxTaskPriorityGet                     ( 1 )
#define INCLUDE_vTaskDelete                           ( 1 )
#define INCLUDE_vTaskSuspend                          ( 1 )
#define INCLUDE_xTaskDelayUntil                       ( 1 )
#define INCLUDE_vTaskDelay                            ( 1 )
#define INCLUDE_xTaskGetSchedulerState                ( 1 )
#define INCLUDE_xTaskGetCurrentTaskHandle             ( 1 )
#define INCLUDE_uxTaskGetStackHighWaterMark           ( 1 )
#define INCLUDE_uxTaskGetStackHighWaterMark2          ( 1 )
#define INCLUDE_xTaskGetIdleTaskHandle                ( 1 )
#define INCLUDE_eTaskGetState                         ( 1 )
#define INCLUDE_xTimerPendFunctionCall                ( 1 )
#define INCLUDE_xTaskAbortDelay                       ( 1 )
#define INCLUDE_xTaskGetHandle                        ( 1 )
#define INCLUDE_xTaskResumeFromISR                    ( 1 )
#define INCLUDE_xQueueGetMutexHolder                  ( 1 )

#define configCOMMAND_INT_MAX_OUTPUT_SIZE             ( 512 )


#endif /* FREERTOS_CONFIG_H */

