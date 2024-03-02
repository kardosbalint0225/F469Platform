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

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "stm32f4xx_hal.h"
#include "hal_errno.h"

#include "led_config.h"
#include "sdcard_config.h"
#include "stdio_uart_config.h"
#include "usbh_conf.h"

EXTI_HandleTypeDef h_exti_sdcard_cd_pin;
EXTI_HandleTypeDef h_exti_usb_host_overcurrent_pin;

typedef struct {
    GPIO_TypeDef *base_address;
    uint32_t reference_count;
    void (*clk_enable_disable_fn)(const clk_state_t state);
} gpio_t;


static gpio_t _gpio[11];

static SemaphoreHandle_t _gpio_mutex = NULL;
static StaticSemaphore_t _gpio_mutex_storage;

static inline void gpio_lock(void);
static inline void gpio_unlock(void);
static int find_port_index(const GPIO_TypeDef *port);
static void gpio_pin_init(const GPIO_TypeDef *port, const GPIO_InitTypeDef *pin);
static void gpio_pin_deinit(const GPIO_TypeDef *port, const uint32_t pin);

void gpio_init(void)
{
    _gpio[0].base_address = GPIOA;
    _gpio[0].reference_count = 0ul;
    _gpio[0].clk_enable_disable_fn = gpio_a_clk_enable_disable;

    _gpio[1].base_address = GPIOB;
    _gpio[1].reference_count = 0ul;
    _gpio[1].clk_enable_disable_fn = gpio_b_clk_enable_disable;

    _gpio[2].base_address = GPIOC;
    _gpio[2].reference_count = 0ul;
    _gpio[2].clk_enable_disable_fn = gpio_c_clk_enable_disable;

    _gpio[3].base_address = GPIOD;
    _gpio[3].reference_count = 0ul;
    _gpio[3].clk_enable_disable_fn = gpio_d_clk_enable_disable;

    _gpio[4].base_address = GPIOE;
    _gpio[4].reference_count = 0ul;
    _gpio[4].clk_enable_disable_fn = gpio_e_clk_enable_disable;

    _gpio[5].base_address = GPIOF;
    _gpio[5].reference_count = 0ul;
    _gpio[5].clk_enable_disable_fn = gpio_f_clk_enable_disable;

    _gpio[6].base_address = GPIOG;
    _gpio[6].reference_count = 0ul;
    _gpio[6].clk_enable_disable_fn = gpio_g_clk_enable_disable;

    _gpio[7].base_address = GPIOH;
    _gpio[7].reference_count = 0ul;
    _gpio[7].clk_enable_disable_fn = gpio_h_clk_enable_disable;

    _gpio[8].base_address = GPIOI;
    _gpio[8].reference_count = 0ul;
    _gpio[8].clk_enable_disable_fn = gpio_i_clk_enable_disable;

    _gpio[9].base_address = GPIOJ;
    _gpio[9].reference_count = 0ul;
    _gpio[9].clk_enable_disable_fn = gpio_j_clk_enable_disable;

    _gpio[10].base_address = GPIOK;
    _gpio[10].reference_count = 0ul;
    _gpio[10].clk_enable_disable_fn = gpio_k_clk_enable_disable;

    _gpio_mutex = xSemaphoreCreateMutexStatic(&_gpio_mutex_storage);
    assert(_gpio_mutex);
}

void gpio_deinit(void)
{
    vSemaphoreDelete(_gpio_mutex);
    _gpio_mutex = NULL;
}

void led1_pin_init(void)
{
    GPIO_InitTypeDef led1 = {
        .Pin = LED1_PIN,
        .Mode = GPIO_MODE_OUTPUT_OD,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW,
    };
    gpio_pin_init(LED1_GPIO_PORT, &led1);
}

void led2_pin_init(void)
{
    GPIO_InitTypeDef led2 = {
        .Pin = LED2_PIN,
        .Mode = GPIO_MODE_OUTPUT_OD,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW,
    };
    gpio_pin_init(LED2_GPIO_PORT, &led2);
}

void led3_pin_init(void)
{
    GPIO_InitTypeDef led3 = {
        .Pin = LED3_PIN,
        .Mode = GPIO_MODE_OUTPUT_OD,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW,
    };
    gpio_pin_init(LED3_GPIO_PORT, &led3);
}

void led4_pin_init(void)
{
    GPIO_InitTypeDef led4 = {
        .Pin = LED4_PIN,
        .Mode = GPIO_MODE_OUTPUT_OD,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW,
    };
    gpio_pin_init(LED4_GPIO_PORT, &led4);
}

void led1_pin_deinit(void)
{
    gpio_pin_deinit(LED1_GPIO_PORT, LED1_PIN);
}

void led2_pin_deinit(void)
{
    gpio_pin_deinit(LED2_GPIO_PORT, LED2_PIN);
}

void led3_pin_deinit(void)
{
    gpio_pin_deinit(LED3_GPIO_PORT, LED3_PIN);
}

void led4_pin_deinit(void)
{
    gpio_pin_deinit(LED4_GPIO_PORT, LED4_PIN);
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
    gpio_pin_init(STDIO_UART_TX_GPIO_PORT, &stdio_uart_tx_pin);
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
    gpio_pin_init(STDIO_UART_RX_GPIO_PORT, &stdio_uart_rx_pin);
}

void stdio_uart_tx_pin_deinit(void)
{
    gpio_pin_deinit(STDIO_UART_TX_GPIO_PORT, STDIO_UART_TX_PIN);
}

void stdio_uart_rx_pin_deinit(void)
{
    gpio_pin_deinit(STDIO_UART_RX_GPIO_PORT, STDIO_UART_RX_PIN);
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
    gpio_pin_init(SDCARD_CMD_PIN_GPIO_PORT, &cmd_pin);
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
    gpio_pin_init(SDCARD_CLK_PIN_GPIO_PORT, &clk_pin);
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
    gpio_pin_init(SDCARD_D3_PIN_GPIO_PORT, &d3_pin);
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
    gpio_pin_init(SDCARD_D2_PIN_GPIO_PORT, &d2_pin);
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
    gpio_pin_init(SDCARD_D1_PIN_GPIO_PORT, &d1_pin);
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
    gpio_pin_init(SDCARD_D0_PIN_GPIO_PORT, &d0_pin);
}

void sdcard_cmd_pin_deinit(void)
{
    gpio_pin_deinit(SDCARD_CMD_PIN_GPIO_PORT, SDCARD_CMD_PIN);
}

void sdcard_clk_pin_deinit(void)
{
    gpio_pin_deinit(SDCARD_CLK_PIN_GPIO_PORT, SDCARD_CLK_PIN);
}

void sdcard_d3_pin_deinit(void)
{
    gpio_pin_deinit(SDCARD_D3_PIN_GPIO_PORT, SDCARD_D3_PIN);
}

void sdcard_d2_pin_deinit(void)
{
    gpio_pin_deinit(SDCARD_D2_PIN_GPIO_PORT, SDCARD_D2_PIN);
}

void sdcard_d1_pin_deinit(void)
{
    gpio_pin_deinit(SDCARD_D1_PIN_GPIO_PORT, SDCARD_D1_PIN);
}

void sdcard_d0_pin_deinit(void)
{
    gpio_pin_deinit(SDCARD_D0_PIN_GPIO_PORT, SDCARD_D0_PIN);
}

int sdcard_cd_pin_init(void (*exti_callback_fn)(void))
{
    assert(exti_callback_fn);

    GPIO_InitTypeDef card_detect_pin_config = {
        .Pin = SDCARD_CD_PIN,
        .Mode = GPIO_MODE_IT_RISING_FALLING,
        .Pull = GPIO_PULLUP,
    };
    gpio_pin_init(SDCARD_CD_PIN_GPIO_PORT, &card_detect_pin_config);

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

    gpio_pin_deinit(SDCARD_CD_PIN_GPIO_PORT, SDCARD_CD_PIN);

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
    gpio_pin_init(USB_HOST_VBUS_PIN_GPIO_PORT, &vbus_pin);
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
    gpio_pin_init(USB_HOST_DP_PIN_GPIO_PORT, &dp_pin);
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
    gpio_pin_init(USB_HOST_DM_PIN_GPIO_PORT, &dm_pin);
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
    gpio_pin_init(USB_HOST_ID_PIN_GPIO_PORT, &id_pin);
}

void usb_host_powerswitch_pin_init(void)
{
    GPIO_InitTypeDef ps_pin = {
       .Pin = USB_HOST_POWERSWITCH_PIN,
       .Mode = GPIO_MODE_OUTPUT_PP,
       .Pull = GPIO_NOPULL,
       .Speed = GPIO_SPEED_FREQ_LOW,
    };
    gpio_pin_init(USB_HOST_POWERSWITCH_PIN_GPIO_PORT, &ps_pin);
}

int usb_host_overcurrent_pin_init(void (*exti_callback_fn)(void))
{
    assert(exti_callback_fn);

    GPIO_InitTypeDef overcurrent_pin_config = {
        .Pin = USB_HOST_OVERCURRENT_PIN,
        .Mode = GPIO_MODE_IT_FALLING,
        .Pull = GPIO_NOPULL,
    };
    gpio_pin_init(USB_HOST_OVERCURRENT_PIN_GPIO_PORT, &overcurrent_pin_config);

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
    gpio_pin_deinit(USB_HOST_VBUS_PIN_GPIO_PORT, USB_HOST_VBUS_PIN);
}

void usb_host_dp_pin_deinit(void)
{
    gpio_pin_deinit(USB_HOST_DP_PIN_GPIO_PORT, USB_HOST_DP_PIN);
}

void usb_host_dm_pin_deinit(void)
{
    gpio_pin_deinit(USB_HOST_DM_PIN_GPIO_PORT, USB_HOST_DM_PIN);
}

void usb_host_id_pin_deinit(void)
{
    gpio_pin_deinit(USB_HOST_ID_PIN_GPIO_PORT, USB_HOST_ID_PIN);
}

void usb_host_powerswitch_pin_deinit(void)
{
    gpio_pin_deinit(USB_HOST_POWERSWITCH_PIN_GPIO_PORT, USB_HOST_POWERSWITCH_PIN);
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

    gpio_pin_deinit(USB_HOST_OVERCURRENT_PIN_GPIO_PORT, USB_HOST_OVERCURRENT_PIN);

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

static inline void gpio_lock(void)
{
    xSemaphoreTake(_gpio_mutex, portMAX_DELAY);
}

static inline void gpio_unlock(void)
{
    xSemaphoreGive(_gpio_mutex);
}

static int find_port_index(const GPIO_TypeDef *port)
{
    int index = -1;
    const int size = sizeof(_gpio) / sizeof(gpio_t);

    for (int i = 0; i < size; i++)
    {
        if (port == _gpio[i].base_address)
        {
            index = i;
            break;
        }
    }

    return index;
}

static void gpio_pin_init(const GPIO_TypeDef *port, const GPIO_InitTypeDef *pin)
{
    assert(pin);
    assert(port);

    gpio_lock();

    const int i = find_port_index(port);
    assert(-1 != i);

    if (0ul == _gpio[i].reference_count)
    {
        _gpio[i].clk_enable_disable_fn(GPIO_CLK_ENABLE);
    }

    HAL_GPIO_Init((GPIO_TypeDef *)_gpio[i].base_address, (GPIO_InitTypeDef *)pin);

    _gpio[i].reference_count = _gpio[i].reference_count + 1ul;

    gpio_unlock();
}

static void gpio_pin_deinit(const GPIO_TypeDef *port, const uint32_t pin)
{
    assert(pin);
    assert(port);

    gpio_lock();

    const int i = find_port_index(port);
    assert(-1 != i);

    HAL_GPIO_DeInit((GPIO_TypeDef *)_gpio[i].base_address, pin);

    _gpio[i].reference_count = _gpio[i].reference_count - 1ul;

    if (0ul == _gpio[i].reference_count)
    {
        _gpio[i].clk_enable_disable_fn(GPIO_CLK_DISABLE);
    }

    gpio_unlock();
}




