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
#include "hal_errno.h"

#include "stdio_uart_config.h"
#include "sdcard_config.h"

EXTI_HandleTypeDef h_exti_sdcard_cd_pin;

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

int sdcard_cd_pin_init(void (*exti_callback_fn)(void))
{
    SDCARD_CD_PIN_GPIO_CLK_ENABLE();

    GPIO_InitTypeDef card_detect_pin_config = {
        .Pin = SDCARD_CD_PIN,
        .Mode = GPIO_MODE_IT_RISING_FALLING,
        .Pull = GPIO_PULLUP,
    };
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

    HAL_NVIC_SetPriority(SDCARD_CD_PIN_EXTIx_IRQn, SDCARD_CD_PIN_EXTIx_IRQ_PRIORITY, 0U);
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

    return 0;
}






