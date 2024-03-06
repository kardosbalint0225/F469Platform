/**
 ******************************************************************************
 * @file    gpio.c
 * @brief   This file provides code for the configuration
 *          of all used GPIO pins.
 ******************************************************************************
 *
 *
 ******************************************************************************
 */
#include "gpio.h"
#include "rcc.h"

#include "stm32f4xx_hal.h"
#include "hal_errno.h"

#include "led_config.h"
#include "sdcard_config.h"
#include "stdio_uart_config.h"
#include "usbh_conf.h"

EXTI_HandleTypeDef h_exti_sdcard_cd_pin;
EXTI_HandleTypeDef h_exti_usb_host_overcurrent_pin;

void led1_pin_init(void)
{
    GPIO_InitTypeDef led1 = {
        .Pin = LED1_PIN,
        .Mode = GPIO_MODE_OUTPUT_OD,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW,
    };
    rcc_periph_clk_enable((const void *)LED1_GPIO_PORT);
    HAL_GPIO_Init(LED1_GPIO_PORT, &led1);
}

void led2_pin_init(void)
{
    GPIO_InitTypeDef led2 = {
        .Pin = LED2_PIN,
        .Mode = GPIO_MODE_OUTPUT_OD,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW,
    };
    rcc_periph_clk_enable((const void *)LED2_GPIO_PORT);
    HAL_GPIO_Init(LED2_GPIO_PORT, &led2);
}

void led3_pin_init(void)
{
    GPIO_InitTypeDef led3 = {
        .Pin = LED3_PIN,
        .Mode = GPIO_MODE_OUTPUT_OD,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW,
    };
    rcc_periph_clk_enable((const void *)LED3_GPIO_PORT);
    HAL_GPIO_Init(LED3_GPIO_PORT, &led3);
}

void led4_pin_init(void)
{
    GPIO_InitTypeDef led4 = {
        .Pin = LED4_PIN,
        .Mode = GPIO_MODE_OUTPUT_OD,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW,
    };
    rcc_periph_clk_enable((const void *)LED4_GPIO_PORT);
    HAL_GPIO_Init(LED4_GPIO_PORT, &led4);
}

void led1_pin_deinit(void)
{
    HAL_GPIO_DeInit(LED1_GPIO_PORT, LED1_PIN);
    rcc_periph_clk_disable((const void *)LED1_GPIO_PORT);
}

void led2_pin_deinit(void)
{
    HAL_GPIO_DeInit(LED2_GPIO_PORT, LED2_PIN);
    rcc_periph_clk_disable((const void *)LED2_GPIO_PORT);
}

void led3_pin_deinit(void)
{
    HAL_GPIO_DeInit(LED3_GPIO_PORT, LED3_PIN);
    rcc_periph_clk_disable((const void *)LED3_GPIO_PORT);
}

void led4_pin_deinit(void)
{
    HAL_GPIO_DeInit(LED4_GPIO_PORT, LED4_PIN);
    rcc_periph_clk_disable((const void *)LED4_GPIO_PORT);
}

void led1_enable(void)
{
    HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_PIN, GPIO_PIN_SET);
}

void led2_enable(void)
{
    HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_SET);
}

void led3_enable(void)
{
    HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_PIN, GPIO_PIN_SET);
}

void led4_enable(void)
{
    HAL_GPIO_WritePin(LED4_GPIO_PORT, LED4_PIN, GPIO_PIN_SET);
}

void led1_disable(void)
{
    HAL_GPIO_WritePin(LED1_GPIO_PORT, LED1_PIN, GPIO_PIN_RESET);
}

void led2_disable(void)
{
    HAL_GPIO_WritePin(LED2_GPIO_PORT, LED2_PIN, GPIO_PIN_RESET);
}

void led3_disable(void)
{
    HAL_GPIO_WritePin(LED3_GPIO_PORT, LED3_PIN, GPIO_PIN_RESET);
}

void led4_disable(void)
{
    HAL_GPIO_WritePin(LED4_GPIO_PORT, LED4_PIN, GPIO_PIN_RESET);
}

void led1_toggle(void)
{
    HAL_GPIO_TogglePin(LED1_GPIO_PORT, LED1_PIN);
}

void led2_toggle(void)
{
    HAL_GPIO_TogglePin(LED2_GPIO_PORT, LED2_PIN);
}

void led3_toggle(void)
{
    HAL_GPIO_TogglePin(LED3_GPIO_PORT, LED3_PIN);
}

void led4_toggle(void)
{
    HAL_GPIO_TogglePin(LED4_GPIO_PORT, LED4_PIN);
}

void stdio_uart_tx_pin_init(void)
{
    GPIO_InitTypeDef stdio_uart_tx_pin = {
        .Pin = STDIO_UART_TX_PIN,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
        .Alternate = STDIO_UART_GPIO_AFx_USARTx,
    };
    rcc_periph_clk_enable((const void *)STDIO_UART_TX_GPIO_PORT);
    HAL_GPIO_Init(STDIO_UART_TX_GPIO_PORT, &stdio_uart_tx_pin);
}

void stdio_uart_rx_pin_init(void)
{
    GPIO_InitTypeDef stdio_uart_rx_pin = {
        .Pin = STDIO_UART_RX_PIN,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
        .Alternate = STDIO_UART_GPIO_AFx_USARTx,
    };
    rcc_periph_clk_enable((const void *)STDIO_UART_RX_GPIO_PORT);
    HAL_GPIO_Init(STDIO_UART_RX_GPIO_PORT, &stdio_uart_rx_pin);
}

void stdio_uart_tx_pin_deinit(void)
{
    HAL_GPIO_DeInit(STDIO_UART_TX_GPIO_PORT, STDIO_UART_TX_PIN);
    rcc_periph_clk_disable((const void *)STDIO_UART_TX_GPIO_PORT);
}

void stdio_uart_rx_pin_deinit(void)
{
    HAL_GPIO_DeInit(STDIO_UART_RX_GPIO_PORT, STDIO_UART_RX_PIN);
    rcc_periph_clk_disable((const void *)STDIO_UART_RX_GPIO_PORT);
}

void sdcard_cmd_pin_init(void)
{
    GPIO_InitTypeDef cmd_pin = {
        .Pin = SDCARD_CMD_PIN,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_PULLUP,
        .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
        .Alternate = SDCARD_GPIO_AFx_SDIO,
    };
    rcc_periph_clk_enable((const void *)SDCARD_CMD_PIN_GPIO_PORT);
    HAL_GPIO_Init(SDCARD_CMD_PIN_GPIO_PORT, &cmd_pin);
}

void sdcard_clk_pin_init(void)
{
    GPIO_InitTypeDef clk_pin = {
        .Pin = SDCARD_CLK_PIN,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_PULLUP,
        .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
        .Alternate = SDCARD_GPIO_AFx_SDIO,
    };
    rcc_periph_clk_enable((const void *)SDCARD_CLK_PIN_GPIO_PORT);
    HAL_GPIO_Init(SDCARD_CLK_PIN_GPIO_PORT, &clk_pin);
}

void sdcard_d3_pin_init(void)
{
    GPIO_InitTypeDef d3_pin = {
       .Pin = SDCARD_D3_PIN,
       .Mode = GPIO_MODE_AF_PP,
       .Pull = GPIO_PULLUP,
       .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
       .Alternate = SDCARD_GPIO_AFx_SDIO,
    };
    rcc_periph_clk_enable((const void *)SDCARD_D3_PIN_GPIO_PORT);
    HAL_GPIO_Init(SDCARD_D3_PIN_GPIO_PORT, &d3_pin);
}

void sdcard_d2_pin_init(void)
{
    GPIO_InitTypeDef d2_pin = {
       .Pin = SDCARD_D2_PIN,
       .Mode = GPIO_MODE_AF_PP,
       .Pull = GPIO_PULLUP,
       .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
       .Alternate = SDCARD_GPIO_AFx_SDIO,
    };
    rcc_periph_clk_enable((const void *)SDCARD_D2_PIN_GPIO_PORT);
    HAL_GPIO_Init(SDCARD_D2_PIN_GPIO_PORT, &d2_pin);
}

void sdcard_d1_pin_init(void)
{
    GPIO_InitTypeDef d1_pin = {
       .Pin = SDCARD_D1_PIN,
       .Mode = GPIO_MODE_AF_PP,
       .Pull = GPIO_PULLUP,
       .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
       .Alternate = SDCARD_GPIO_AFx_SDIO,
    };
    rcc_periph_clk_enable((const void *)SDCARD_D1_PIN_GPIO_PORT);
    HAL_GPIO_Init(SDCARD_D1_PIN_GPIO_PORT, &d1_pin);
}

void sdcard_d0_pin_init(void)
{
    GPIO_InitTypeDef d0_pin = {
       .Pin = SDCARD_D0_PIN,
       .Mode = GPIO_MODE_AF_PP,
       .Pull = GPIO_PULLUP,
       .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
       .Alternate = SDCARD_GPIO_AFx_SDIO,
    };
    rcc_periph_clk_enable((const void *)SDCARD_D0_PIN_GPIO_PORT);
    HAL_GPIO_Init(SDCARD_D0_PIN_GPIO_PORT, &d0_pin);
}

void sdcard_cmd_pin_deinit(void)
{
    HAL_GPIO_DeInit(SDCARD_CMD_PIN_GPIO_PORT, SDCARD_CMD_PIN);
    rcc_periph_clk_disable((const void *)SDCARD_CMD_PIN_GPIO_PORT);
}

void sdcard_clk_pin_deinit(void)
{
    HAL_GPIO_DeInit(SDCARD_CLK_PIN_GPIO_PORT, SDCARD_CLK_PIN);
    rcc_periph_clk_disable((const void *)SDCARD_CLK_PIN_GPIO_PORT);
}

void sdcard_d3_pin_deinit(void)
{
    HAL_GPIO_DeInit(SDCARD_D3_PIN_GPIO_PORT, SDCARD_D3_PIN);
    rcc_periph_clk_disable((const void *)SDCARD_D3_PIN_GPIO_PORT);
}

void sdcard_d2_pin_deinit(void)
{
    HAL_GPIO_DeInit(SDCARD_D2_PIN_GPIO_PORT, SDCARD_D2_PIN);
    rcc_periph_clk_disable((const void *)SDCARD_D2_PIN_GPIO_PORT);
}

void sdcard_d1_pin_deinit(void)
{
    HAL_GPIO_DeInit(SDCARD_D1_PIN_GPIO_PORT, SDCARD_D1_PIN);
    rcc_periph_clk_disable((const void *)SDCARD_D1_PIN_GPIO_PORT);
}

void sdcard_d0_pin_deinit(void)
{
    HAL_GPIO_DeInit(SDCARD_D0_PIN_GPIO_PORT, SDCARD_D0_PIN);
    rcc_periph_clk_disable((const void *)SDCARD_D0_PIN_GPIO_PORT);
}

int sdcard_cd_pin_init(void (*exti_callback_fn)(void))
{
    assert(exti_callback_fn);

    GPIO_InitTypeDef card_detect_pin_config = {
        .Pin = SDCARD_CD_PIN,
        .Mode = GPIO_MODE_IT_RISING_FALLING,
        .Pull = GPIO_PULLUP,
    };
    rcc_periph_clk_enable((const void *)SDCARD_CD_PIN_GPIO_PORT);
    HAL_GPIO_Init(SDCARD_CD_PIN_GPIO_PORT, &card_detect_pin_config);

    EXTI_ConfigTypeDef exti_config = {
        .Line = SDCARD_CD_PIN_EXTI_LINE,
        .Mode = EXTI_MODE_INTERRUPT,
        .Trigger = EXTI_TRIGGER_RISING_FALLING,
        .GPIOSel = SDCARD_CD_PIN_EXTI_GPIO
    };

    HAL_StatusTypeDef ret;
    ret = HAL_EXTI_SetConfigLine(&h_exti_sdcard_cd_pin, &exti_config);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_EXTI_RegisterCallback(&h_exti_sdcard_cd_pin, HAL_EXTI_COMMON_CB_ID, exti_callback_fn);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    HAL_NVIC_SetPriority(SDCARD_CD_PIN_EXTIx_IRQn, SDCARD_CD_PIN_EXTIx_IRQ_PRIORITY, 0ul);
    HAL_NVIC_EnableIRQ(SDCARD_CD_PIN_EXTIx_IRQn);

    return 0;
}

int sdcard_cd_pin_deinit(void)
{
    HAL_StatusTypeDef ret;

    HAL_NVIC_DisableIRQ(SDCARD_CD_PIN_EXTIx_IRQn);

    ret = HAL_EXTI_ClearConfigLine(&h_exti_sdcard_cd_pin);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    HAL_GPIO_DeInit(SDCARD_CD_PIN_GPIO_PORT, SDCARD_CD_PIN);
    rcc_periph_clk_disable((const void *)SDCARD_CD_PIN_GPIO_PORT);

    return 0;
}

void usb_host_vbus_pin_init(void)
{
    GPIO_InitTypeDef vbus_pin = {
       .Pin = USB_HOST_VBUS_PIN,
       .Mode = GPIO_MODE_AF_PP,
       .Pull = GPIO_NOPULL,
       .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
       .Alternate = USB_HOST_GPIO_AFx_OTG_FS,
    };
    rcc_periph_clk_enable((const void *)USB_HOST_VBUS_PIN_GPIO_PORT);
    HAL_GPIO_Init(USB_HOST_VBUS_PIN_GPIO_PORT, &vbus_pin);
}

void usb_host_dp_pin_init(void)
{
    GPIO_InitTypeDef dp_pin = {
       .Pin = USB_HOST_DP_PIN,
       .Mode = GPIO_MODE_AF_PP,
       .Pull = GPIO_NOPULL,
       .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
       .Alternate = USB_HOST_GPIO_AFx_OTG_FS,
    };
    rcc_periph_clk_enable((const void *)USB_HOST_DP_PIN_GPIO_PORT);
    HAL_GPIO_Init(USB_HOST_DP_PIN_GPIO_PORT, &dp_pin);
}

void usb_host_dm_pin_init(void)
{
    GPIO_InitTypeDef dm_pin = {
       .Pin = USB_HOST_DM_PIN,
       .Mode = GPIO_MODE_AF_PP,
       .Pull = GPIO_NOPULL,
       .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
       .Alternate = USB_HOST_GPIO_AFx_OTG_FS,
    };
    rcc_periph_clk_enable((const void *)USB_HOST_DM_PIN_GPIO_PORT);
    HAL_GPIO_Init(USB_HOST_DM_PIN_GPIO_PORT, &dm_pin);
}

void usb_host_id_pin_init(void)
{
    GPIO_InitTypeDef id_pin = {
       .Pin = USB_HOST_ID_PIN,
       .Mode = GPIO_MODE_AF_PP,
       .Pull = GPIO_NOPULL,
       .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
       .Alternate = USB_HOST_GPIO_AFx_OTG_FS,
    };
    rcc_periph_clk_enable((const void *)USB_HOST_ID_PIN_GPIO_PORT);
    HAL_GPIO_Init(USB_HOST_ID_PIN_GPIO_PORT, &id_pin);
}

void usb_host_powerswitch_pin_init(void)
{
    GPIO_InitTypeDef ps_pin = {
       .Pin = USB_HOST_POWERSWITCH_PIN,
       .Mode = GPIO_MODE_OUTPUT_PP,
       .Pull = GPIO_NOPULL,
       .Speed = GPIO_SPEED_FREQ_LOW,
    };
    rcc_periph_clk_enable((const void *)USB_HOST_POWERSWITCH_PIN_GPIO_PORT);
    HAL_GPIO_Init(USB_HOST_POWERSWITCH_PIN_GPIO_PORT, &ps_pin);
}

int usb_host_overcurrent_pin_init(void (*exti_callback_fn)(void))
{
    assert(exti_callback_fn);

    GPIO_InitTypeDef overcurrent_pin_config = {
        .Pin = USB_HOST_OVERCURRENT_PIN,
        .Mode = GPIO_MODE_IT_FALLING,
        .Pull = GPIO_NOPULL,
    };
    rcc_periph_clk_enable((const void *)USB_HOST_OVERCURRENT_PIN_GPIO_PORT);
    HAL_GPIO_Init(USB_HOST_OVERCURRENT_PIN_GPIO_PORT, &overcurrent_pin_config);

    EXTI_ConfigTypeDef exti_config = {
        .Line = USB_HOST_OVERCURRENT_PIN_EXTI_LINE,
        .Mode = EXTI_MODE_INTERRUPT,
        .Trigger = EXTI_TRIGGER_FALLING,
        .GPIOSel = USB_HOST_OVERCURRENT_PIN_EXTI_GPIO
    };

    HAL_StatusTypeDef ret;
    ret = HAL_EXTI_SetConfigLine(&h_exti_usb_host_overcurrent_pin, &exti_config);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    ret = HAL_EXTI_RegisterCallback(&h_exti_usb_host_overcurrent_pin, HAL_EXTI_COMMON_CB_ID, exti_callback_fn);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    HAL_NVIC_SetPriority(USB_HOST_OVERCURRENT_PIN_EXTIx_IRQn, USB_HOST_OVERCURRENT_PIN_EXTIx_IRQ_PRIORITY, 0ul);
    HAL_NVIC_EnableIRQ(USB_HOST_OVERCURRENT_PIN_EXTIx_IRQn);

    return 0;
}

void usb_host_vbus_pin_deinit(void)
{
    HAL_GPIO_DeInit(USB_HOST_VBUS_PIN_GPIO_PORT, USB_HOST_VBUS_PIN);
    rcc_periph_clk_disable((const void *)USB_HOST_VBUS_PIN_GPIO_PORT);
}

void usb_host_dp_pin_deinit(void)
{
    HAL_GPIO_DeInit(USB_HOST_DP_PIN_GPIO_PORT, USB_HOST_DP_PIN);
    rcc_periph_clk_disable((const void *)USB_HOST_DP_PIN_GPIO_PORT);
}

void usb_host_dm_pin_deinit(void)
{
    HAL_GPIO_DeInit(USB_HOST_DM_PIN_GPIO_PORT, USB_HOST_DM_PIN);
    rcc_periph_clk_disable((const void *)USB_HOST_DM_PIN_GPIO_PORT);
}

void usb_host_id_pin_deinit(void)
{
    HAL_GPIO_DeInit(USB_HOST_ID_PIN_GPIO_PORT, USB_HOST_ID_PIN);
    rcc_periph_clk_disable((const void *)USB_HOST_ID_PIN_GPIO_PORT);
}

void usb_host_powerswitch_pin_deinit(void)
{
    HAL_GPIO_DeInit(USB_HOST_POWERSWITCH_PIN_GPIO_PORT, USB_HOST_POWERSWITCH_PIN);
    rcc_periph_clk_disable((const void *)USB_HOST_POWERSWITCH_PIN_GPIO_PORT);
}

int usb_host_overcurrent_pin_deinit(void)
{
    HAL_StatusTypeDef ret;

    HAL_NVIC_DisableIRQ(USB_HOST_OVERCURRENT_PIN_EXTIx_IRQn);   //TODO: EXTI9_5_IRQn is a shared interrupt line

    ret = HAL_EXTI_ClearConfigLine(&h_exti_usb_host_overcurrent_pin);
    if (HAL_OK != ret)
    {
        return hal_statustypedef_to_errno(ret);
    }

    HAL_GPIO_DeInit(USB_HOST_OVERCURRENT_PIN_GPIO_PORT, USB_HOST_OVERCURRENT_PIN);
    rcc_periph_clk_disable((const void *)USB_HOST_OVERCURRENT_PIN_GPIO_PORT);

    return 0;
}

void usb_host_powerswitch_enable(void)
{
    HAL_GPIO_WritePin(USB_HOST_POWERSWITCH_PIN_GPIO_PORT, USB_HOST_POWERSWITCH_PIN, GPIO_PIN_SET);
}

void usb_host_powerswitch_disable(void)
{
    HAL_GPIO_WritePin(USB_HOST_POWERSWITCH_PIN_GPIO_PORT, USB_HOST_POWERSWITCH_PIN, GPIO_PIN_RESET);
}




