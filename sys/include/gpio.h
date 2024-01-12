/**
 ******************************************************************************
 * @file    gpio.h
 * @brief   This file contains all the function prototypes for
 *          the gpio.c file
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
#ifndef __GPIO_H__
#define __GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

void stdio_uart_tx_pin_init(void);
void stdio_uart_rx_pin_init(void);
void stdio_uart_tx_pin_deinit(void);
void stdio_uart_rx_pin_deinit(void);

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

#ifdef __cplusplus
}
#endif
#endif /*__ GPIO_H__ */

