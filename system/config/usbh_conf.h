/*
 * usb_host_config.h
 *
 *  Created on: Jan 9, 2024
 *      Author: Balint
 */

#ifndef __USBH_CONF__H__
#define __USBH_CONF__H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define USB_HOST_VBUS_PIN                           GPIO_PIN_9
#define USB_HOST_VBUS_PIN_GPIO_PORT                 GPIOA
#define USB_HOST_DP_PIN                             GPIO_PIN_12
#define USB_HOST_DP_PIN_GPIO_PORT                   GPIOA
#define USB_HOST_DM_PIN                             GPIO_PIN_11
#define USB_HOST_DM_PIN_GPIO_PORT                   GPIOA
#define USB_HOST_ID_PIN                             GPIO_PIN_10
#define USB_HOST_ID_PIN_GPIO_PORT                   GPIOA
#define USB_HOST_POWERSWITCH_PIN                    GPIO_PIN_2
#define USB_HOST_POWERSWITCH_PIN_GPIO_PORT          GPIOB
#define USB_HOST_OVERCURRENT_PIN                    GPIO_PIN_7
#define USB_HOST_OVERCURRENT_PIN_GPIO_PORT          GPIOB
#define USB_HOST_OVERCURRENT_PIN_EXTI_GPIO          EXTI_GPIOB
#define USB_HOST_OVERCURRENT_PIN_EXTI_LINE          EXTI_LINE_7
#define USB_HOST_OVERCURRENT_PIN_EXTIx_IRQn         EXTI9_5_IRQn
#define USB_HOST_OVERCURRENT_PIN_EXTIx_IRQ_PRIORITY 6
#define USB_HOST_OVERCURRENT_PIN_EXTIx_IRQHandler   EXTI9_5_IRQHandler
#define USB_HOST_GPIO_AFx_OTG_FS                    GPIO_AF10_OTG_FS

#define USB_HOST_TASK_STACK_SIZE          (31 * configMINIMAL_STACK_SIZE + configMINIMAL_STACK_SIZE)
#define USB_HOST_TASK_PRIORITY            2

#define USBH_MAX_NUM_ENDPOINTS            2U
#define USBH_MAX_NUM_INTERFACES           2U
#define USBH_MAX_NUM_CONFIGURATION        1U
#define USBH_KEEP_CFG_DESCRIPTOR          1U
#define USBH_MAX_NUM_SUPPORTED_CLASS      1U
#define USBH_MAX_SIZE_CONFIGURATION       256U
#define USBH_MAX_DATA_BUFFER              512U
#define USBH_DEBUG_LEVEL                  4U
#define USBH_USE_OS                       1U

#define HOST_HS                           0
#define HOST_FS                           1

#define USBH_malloc                       pvPortMalloc
#define USBH_free                         vPortFree
#define USBH_memset                       memset
#define USBH_memcpy                       memcpy

#if (USBH_DEBUG_LEVEL > 0U)
#define  USBH_UsrLog(...)   do { \
                            printf(__VA_ARGS__); \
                            printf("\n"); \
} while (0)
#else
#define USBH_UsrLog(...) do {} while (0)
#endif

#if (USBH_DEBUG_LEVEL > 1U)

#define  USBH_ErrLog(...) do { \
                            printf("ERROR: ") ; \
                            printf(__VA_ARGS__); \
                            printf("\n"); \
} while (0)
#else
#define USBH_ErrLog(...) do {} while (0)
#endif

#if (USBH_DEBUG_LEVEL > 2U)
#define  USBH_DbgLog(...)   do { \
                            printf("DEBUG : ") ; \
                            printf(__VA_ARGS__); \
                            printf("\n"); \
} while (0)
#else
#define USBH_DbgLog(...) do {} while (0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __USBH_CONF__H__ */

