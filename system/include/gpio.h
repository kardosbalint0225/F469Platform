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
 * @ingroup     system_gpio
 *
 * @file        gpio.h
 * @brief       GPIO Management for the peripherals used by the application
 */
#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup LED_GPIO_Functions LED GPIO Functions
 *  @brief Functions for controlling LEDs.
 *  @{
 */
void led1_pin_init(void);
void led2_pin_init(void);
void led3_pin_init(void);
void led4_pin_init(void);
void led1_pin_deinit(void);
void led2_pin_deinit(void);
void led3_pin_deinit(void);
void led4_pin_deinit(void);
void led1_enable(void);
void led2_enable(void);
void led3_enable(void);
void led4_enable(void);
void led1_disable(void);
void led2_disable(void);
void led3_disable(void);
void led4_disable(void);
void led1_toggle(void);
void led2_toggle(void);
void led3_toggle(void);
void led4_toggle(void);
/** @} */

/** @defgroup STDIO_UART_GPIO_Functions STDIO UART GPIO Management Functions
 *  @brief Functions related to Tx and Rx GPIO operations of the STDIO UART.
 *  @{
 */
void stdio_uart_tx_pin_init(void);
void stdio_uart_rx_pin_init(void);
void stdio_uart_tx_pin_deinit(void);
void stdio_uart_rx_pin_deinit(void);
/** @} */

/** @defgroup SDCard_GPIO_Functions SD Card GPIO Management Functions
 *  @brief Functions related to CLK, CMD and Data line GPIO operations of the SD Card.
 *  @{
 */
void sdcard_cmd_pin_init(void);
void sdcard_clk_pin_init(void);
void sdcard_d3_pin_init(void);
void sdcard_d2_pin_init(void);
void sdcard_d1_pin_init(void);
void sdcard_d0_pin_init(void);
void sdcard_cmd_pin_deinit(void);
void sdcard_clk_pin_deinit(void);
void sdcard_d3_pin_deinit(void);
void sdcard_d2_pin_deinit(void);
void sdcard_d1_pin_deinit(void);
void sdcard_d0_pin_deinit(void);
int  sdcard_cd_pin_init(void (*exti_callback_fn)(void));
int  sdcard_cd_pin_deinit(void);
/** @} */

/** @defgroup USB_Host_GPIO_Functions USB Host GPIO Management Functions
 *  @brief Functions related to Full-Speed USB Host GPIO operations.
 *  @{
 */
void usb_host_vbus_pin_init(void);
void usb_host_dp_pin_init(void);
void usb_host_dm_pin_init(void);
void usb_host_id_pin_init(void);
void usb_host_powerswitch_pin_init(void);
int  usb_host_overcurrent_pin_init(void (*exti_callback_fn)(void));
void usb_host_vbus_pin_deinit(void);
void usb_host_dp_pin_deinit(void);
void usb_host_dm_pin_deinit(void);
void usb_host_id_pin_deinit(void);
void usb_host_powerswitch_pin_deinit(void);
int  usb_host_overcurrent_pin_deinit(void);
void usb_host_powerswitch_enable(void);
void usb_host_powerswitch_disable(void);
/** @} */

#ifdef __cplusplus
}
#endif
#endif /*__ GPIO_H__ */

