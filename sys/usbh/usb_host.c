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
#include <errno.h>

#include "usb_host.h"
#include "usbh_core.h"
#include "usbh_msc.h"
#include "usbh_conf.h"

USBH_HandleTypeDef h_usb_host;

static void usb_host_event_callback(USBH_HandleTypeDef *phost, uint8_t id);
static int usbh_status_to_errno(const USBH_StatusTypeDef status);

int usb_host_init(void)
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

int usb_host_deinit(void)
{
    return 0;
}

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


