/*
 * usbh_monitor.c
 *
 *  Created on: Jan 4, 2024
 *      Author: Balint
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <assert.h>

#include "usb_host.h"
#include "tusb.h"
#include "tusb_config.h"

static StackType_t _usb_host_task_stack[USB_HOST_TASK_STACK_SIZE];
static StaticTask_t _usb_host_task_tcb;
static TaskHandle_t h_usb_host_task = NULL;

static void usb_host_task(void *params);

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len);

int usb_host_init(void)
{
    h_usb_host_task = xTaskCreateStatic(usb_host_task,
                                        "USB Host",
                                        USB_HOST_TASK_STACK_SIZE,
                                        NULL,
                                        USB_HOST_TASK_PRIORITY,
                                        _usb_host_task_stack,
                                        &_usb_host_task_tcb);
    assert(h_usb_host_task);

    tusb_init();
    return 0;
}

int usb_host_deinit(void)
{
    return 0;
}

static void usb_host_task(void *params)
{
    (void)params;

    tuh_init(BOARD_TUH_RHPORT);

    while (1)
    {
        tuh_task();
    }
}

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len)
{

}

