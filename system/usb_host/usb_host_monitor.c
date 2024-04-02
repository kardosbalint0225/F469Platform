/*
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
 * @ingroup     system_usb_host_monitor
 * @{
 * @file        usb_host_monitor.c
 * @brief       USB Host Monitor
 */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include <errno.h>

#include "usb_host_monitor.h"
#include "usbh_core.h"
#include "usbh_msc.h"
#include "usbh_conf.h"

static USBH_HandleTypeDef h_usb_host;

static void usb_host_event_callback(USBH_HandleTypeDef *phost, uint8_t id);
static int usbh_status_to_errno(const USBH_StatusTypeDef status);

int usb_host_monitor_init(void)
{
    USBH_StatusTypeDef ret;

    ret = USBH_Init(&h_usb_host, usb_host_event_callback, HOST_FS);
    if (ret != USBH_OK)
    {
        return usbh_status_to_errno(ret);
    }

    ret = USBH_RegisterClass(&h_usb_host, USBH_MSC_CLASS);
    if (ret != USBH_OK)
    {
        return usbh_status_to_errno(ret);
    }

    ret = USBH_Start(&h_usb_host);
    if (ret != USBH_OK)
    {
        return usbh_status_to_errno(ret);
    }

    return 0;
}

int usb_host_monitor_deinit(void)
{
    return 0;
}

/**
 * @brief USB Host Event Callback.
 *
 * @param phost Pointer to the USBH_HandleTypeDef structure.
 * @param id    Identifier of the USB Host Event.
 */
static void usb_host_event_callback(USBH_HandleTypeDef *phost, uint8_t id)
{
    switch (id)
    {
        case HOST_USER_SELECT_CONFIGURATION :
            printf("USBH_UserProcess: Select configuration\r\n");
        break;

        case HOST_USER_CLASS_ACTIVE         :
            printf("USBH_UserProcess: Class active\r\n");
        break;

        case HOST_USER_CLASS_SELECTED       :
            printf("USBH_UserProcess: Class selected\r\n");
        break;

        case HOST_USER_CONNECTION           :
            printf("USBH_UserProcess: Connection\r\n");
        break;

        case HOST_USER_DISCONNECTION        :
            printf("USBH_UserProcess: Disconnection\r\n");
        break;

        case HOST_USER_UNRECOVERED_ERROR    :
            printf("USBH_UserProcess: Unrecovered error\r\n");
        break;

        default                             :
        break;
    }
}

/**
 * @brief Convert USBH status to errno.
 *
 * This function maps USBH_StatusTypeDef values to corresponding POSIX errno codes.
 *
 * @param  status USBH status to be converted.
 * @return Corresponding POSIX errno code.
 */
static int usbh_status_to_errno(const USBH_StatusTypeDef status)
{
    switch (status)
    {
        case USBH_OK                  : return 0;
        case USBH_BUSY                : return -EBUSY;
        case USBH_FAIL                : return -EIO;
        case USBH_NOT_SUPPORTED       : return -ENOTSUP;
        case USBH_UNRECOVERED_ERROR   : return -EIO;
        case USBH_ERROR_SPEED_UNKNOWN : return -EIO;
        default                       : return -EIO;
    }

    return (int)status;
}
/** @} */

