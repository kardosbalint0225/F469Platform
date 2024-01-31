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

