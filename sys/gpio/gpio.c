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
#include "stm32f4xx_hal.h"

#include "stdio_uart_config.h"
#include "sdcard_config.h"

void stdio_uart_tx_pin_init(void)
{
    GPIO_InitTypeDef stdio_uart_tx_pin = {
        .Pin = STDIO_UART_TX_PIN,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
        .Alternate = STDIO_UART_GPIO_AFx_USARTx,
    };

    STDIO_UART_TX_PIN_GPIOx_CLK_ENABLE();
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

    STDIO_UART_RX_PIN_GPIOx_CLK_ENABLE();
    HAL_GPIO_Init(STDIO_UART_RX_GPIO_PORT, &stdio_uart_rx_pin);
}

void stdio_uart_tx_pin_deinit(void)
{
    HAL_GPIO_DeInit(STDIO_UART_TX_GPIO_PORT, STDIO_UART_TX_PIN);
}

void stdio_uart_rx_pin_deinit(void)
{
    HAL_GPIO_DeInit(STDIO_UART_RX_GPIO_PORT, STDIO_UART_RX_PIN);
}

void sdcard_cmd_pin_init(void)
{
    SDCARD_CMD_PIN_GPIO_CLK_ENABLE();

    GPIO_InitTypeDef cmd_pin = {
        .Pin = SDCARD_CMD_PIN,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_PULLUP,
        .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
        .Alternate = SDCARD_GPIO_AFx_SDIO,
    };
    HAL_GPIO_Init(SDCARD_CMD_PIN_GPIO_PORT, &cmd_pin);
}

void sdcard_clk_pin_init(void)
{
    SDCARD_CLK_PIN_GPIO_CLK_ENABLE();

    GPIO_InitTypeDef clk_pin = {
        .Pin = SDCARD_CLK_PIN,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_PULLUP,
        .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
        .Alternate = SDCARD_GPIO_AFx_SDIO,
    };
    HAL_GPIO_Init(SDCARD_CLK_PIN_GPIO_PORT, &clk_pin);
}

void sdcard_d3_pin_init(void)
{
    SDCARD_D3_PIN_GPIO_CLK_ENABLE();

    GPIO_InitTypeDef d3_pin = {
       .Pin = SDCARD_D3_PIN,
       .Mode = GPIO_MODE_AF_PP,
       .Pull = GPIO_PULLUP,
       .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
       .Alternate = SDCARD_GPIO_AFx_SDIO,
    };
    HAL_GPIO_Init(SDCARD_D3_PIN_GPIO_PORT, &d3_pin);
}

void sdcard_d2_pin_init(void)
{
    SDCARD_D2_PIN_GPIO_CLK_ENABLE();

    GPIO_InitTypeDef d2_pin = {
       .Pin = SDCARD_D2_PIN,
       .Mode = GPIO_MODE_AF_PP,
       .Pull = GPIO_PULLUP,
       .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
       .Alternate = SDCARD_GPIO_AFx_SDIO,
    };
    HAL_GPIO_Init(SDCARD_D2_PIN_GPIO_PORT, &d2_pin);
}

void sdcard_d1_pin_init(void)
{
    SDCARD_D1_PIN_GPIO_CLK_ENABLE();

    GPIO_InitTypeDef d1_pin = {
       .Pin = SDCARD_D1_PIN,
       .Mode = GPIO_MODE_AF_PP,
       .Pull = GPIO_PULLUP,
       .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
       .Alternate = SDCARD_GPIO_AFx_SDIO,
    };
    HAL_GPIO_Init(SDCARD_D1_PIN_GPIO_PORT, &d1_pin);
}

void sdcard_d0_pin_init(void)
{
    SDCARD_D0_PIN_GPIO_CLK_ENABLE();

    GPIO_InitTypeDef d0_pin = {
       .Pin = SDCARD_D0_PIN,
       .Mode = GPIO_MODE_AF_PP,
       .Pull = GPIO_PULLUP,
       .Speed = GPIO_SPEED_FREQ_VERY_HIGH,
       .Alternate = SDCARD_GPIO_AFx_SDIO,
    };
    HAL_GPIO_Init(SDCARD_D0_PIN_GPIO_PORT, &d0_pin);
}

void sdcard_cmd_pin_deinit(void)
{
    HAL_GPIO_DeInit(SDCARD_CMD_PIN_GPIO_PORT, SDCARD_CMD_PIN);
}

void sdcard_clk_pin_deinit(void)
{
    HAL_GPIO_DeInit(SDCARD_CLK_PIN_GPIO_PORT, SDCARD_CLK_PIN);
}

void sdcard_d3_pin_deinit(void)
{
    HAL_GPIO_DeInit(SDCARD_D3_PIN_GPIO_PORT, SDCARD_D3_PIN);
}

void sdcard_d2_pin_deinit(void)
{
    HAL_GPIO_DeInit(SDCARD_D2_PIN_GPIO_PORT, SDCARD_D2_PIN);
}

void sdcard_d1_pin_deinit(void)
{
    HAL_GPIO_DeInit(SDCARD_D1_PIN_GPIO_PORT, SDCARD_D1_PIN);
}

void sdcard_d0_pin_deinit(void)
{
    HAL_GPIO_DeInit(SDCARD_D0_PIN_GPIO_PORT, SDCARD_D0_PIN);
}








