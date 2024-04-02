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
 * @file        usb_host_monitor.h
 * @brief       USB Host Monitor
 */
#ifndef __USB_HOST_MONITOR_H__
#define __USB_HOST_MONITOR_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initializes the USB Host Monitor.
 *
 * This function initializes the USB host controller and registers the Mass Storage Class
 * (MSC) driver. It then starts the USB host monitor task.
 *
 * @return 0 on success,
 * @return < 0 on error.
 */
int usb_host_monitor_init(void);

/**
 * @brief De-initializes the USB Host Monitor.
 *
 * TODO:
 *
 * @return 0 on success,
 * @return < 0 on error.
 */
int usb_host_monitor_deinit(void);

#ifdef __cplusplus
}
#endif
#endif /* __USB_HOST_MONITOR_H__ */
/** @} */
