/*
 * hcd.c
 *
 *  Created on: Jan 9, 2024
 *      Author: Balint
 */
#include "rcc.h"
#include "gpio.h"
#include "usbh_conf.h"
#include "usbh_core.h"

HCD_HandleTypeDef h_hcd_fs;
static HAL_StatusTypeDef _error = HAL_OK;

static void hcd_msp_init(HCD_HandleTypeDef *hhcd);
static void hcd_msp_deinit(HCD_HandleTypeDef *hhcd);
static void hcd_sof_callback(HCD_HandleTypeDef *hhcd);
static void hcd_connect_callback(HCD_HandleTypeDef *hhcd);
static void hcd_disconnect_callback(HCD_HandleTypeDef *hhcd);
static void hcd_hc_notify_urb_change_callback(HCD_HandleTypeDef *hhcd, uint8_t chnum, HCD_URBStateTypeDef urb_state);
static void hcd_port_enabled_callback(HCD_HandleTypeDef *hhcd);
static void hcd_port_disabled_callback(HCD_HandleTypeDef *hhcd);
static USBH_StatusTypeDef hal_status_to_usbh_status(HAL_StatusTypeDef hal_status);

static void hcd_msp_init(HCD_HandleTypeDef *hhcd)
{
//    RCC_PeriphCLKInitTypeDef RCC_PeriphClkInitStruct;
//    RCC_PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CLK48;
//    RCC_PeriphClkInitStruct.SdioClockSelection = RCC_SDIOCLKSOURCE_CLK48;
//    RCC_PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48CLKSOURCE_PLLSAIP;
//    RCC_PeriphClkInitStruct.PLLSAI.PLLSAIN = 384;
//    RCC_PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV8;
    _error = clk48_clock_init();//HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInitStruct);
    if (HAL_OK != _error)
    {
        return;
    }

    __HAL_RCC_USB_OTG_FS_CLK_ENABLE();
    __HAL_RCC_USB_OTG_FS_FORCE_RESET();
    __HAL_RCC_USB_OTG_FS_RELEASE_RESET();

    usb_host_powerswitch_pin_init();
    usb_host_dp_pin_init();
    usb_host_dm_pin_init();
    usb_host_id_pin_init();
    usb_host_vbus_pin_init();

    HAL_NVIC_SetPriority(OTG_FS_IRQn, 7, 0);
    HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
}

static void hcd_msp_deinit(HCD_HandleTypeDef *hhcd)
{
    _error = clk48_clock_deinit();
    if (HAL_OK != _error)
    {
        return;
    }

    __HAL_RCC_USB_OTG_FS_CLK_DISABLE();
    HAL_NVIC_DisableIRQ(OTG_FS_IRQn);

    usb_host_dp_pin_deinit();
    usb_host_dm_pin_deinit();
    usb_host_id_pin_deinit();
    usb_host_vbus_pin_deinit();
    usb_host_powerswitch_disable();
    usb_host_powerswitch_pin_deinit();
}

/**
  * @brief  SOF callback.
  * @param  hhcd: HCD handle
  * @retval None
  */
static void hcd_sof_callback(HCD_HandleTypeDef *hhcd)
{
    USBH_LL_IncTimer(hhcd->pData);
}

/**
  * @brief  Connect callback.
  * @param  hhcd: HCD handle
  * @retval None
  */
static void hcd_connect_callback(HCD_HandleTypeDef *hhcd)
{
    USBH_LL_Connect(hhcd->pData);
}

/**
  * @brief  Disconnect callback.
  * @param  hhcd: HCD handle
  * @retval None
  */
static void hcd_disconnect_callback(HCD_HandleTypeDef *hhcd)
{
    USBH_LL_Disconnect(hhcd->pData);
}

/**
  * @brief  Notify URB state change callback.
  * @param  hhcd: HCD handle
  * @param  chnum: channel number
  * @param  urb_state: state
  * @retval None
  */
static void hcd_hc_notify_urb_change_callback(HCD_HandleTypeDef *hhcd, uint8_t chnum, HCD_URBStateTypeDef urb_state)
{
#if (USBH_USE_OS == 1)
    USBH_LL_NotifyURBChange(hhcd->pData);
#endif
}
/**
  * @brief  Port Enabled callback.
  * @param  hhcd: HCD handle
  * @retval None
  */
static void hcd_port_enabled_callback(HCD_HandleTypeDef *hhcd)
{
    USBH_LL_PortEnabled(hhcd->pData);
}

/**
  * @brief  Port Disabled callback.
  * @param  hhcd: HCD handle
  * @retval None
  */
static void hcd_port_disabled_callback(HCD_HandleTypeDef *hhcd)
{
    USBH_LL_PortDisabled(hhcd->pData);
}

/**
  * @brief  Initialize the low level portion of the host driver.
  * @param  phost: Host handle
  * @retval USBH status
  */
USBH_StatusTypeDef USBH_LL_Init(USBH_HandleTypeDef *phost)
{
    _error = HAL_OK;

    h_hcd_fs.pData = phost;
    phost->pData = &h_hcd_fs;

    h_hcd_fs.Instance = USB_OTG_FS;
    h_hcd_fs.Init.Host_channels = 8;
    h_hcd_fs.Init.speed = HCD_SPEED_FULL;
    h_hcd_fs.Init.dma_enable = DISABLE;
    h_hcd_fs.Init.phy_itface = HCD_PHY_EMBEDDED;
    h_hcd_fs.Init.Sof_enable = DISABLE;

    HAL_StatusTypeDef ret;

    ret = HAL_HCD_RegisterCallback(&h_hcd_fs, HAL_HCD_MSPINIT_CB_ID, hcd_msp_init);
    if (ret != HAL_OK)
    {
        return hal_status_to_usbh_status(ret);
    }

    ret = HAL_HCD_RegisterCallback(&h_hcd_fs, HAL_HCD_MSPDEINIT_CB_ID, hcd_msp_deinit);
    if (ret != HAL_OK)
    {
        return hal_status_to_usbh_status(ret);
    }

    ret = HAL_HCD_Init(&h_hcd_fs);
    if (ret != HAL_OK)
    {
        return hal_status_to_usbh_status(ret);
    }

    if (_error != HAL_OK)
    {
        return hal_status_to_usbh_status(_error);
    }

    ret = HAL_HCD_RegisterCallback(&h_hcd_fs, HAL_HCD_SOF_CB_ID, hcd_sof_callback);
    if (ret != HAL_OK)
    {
        return hal_status_to_usbh_status(ret);
    }

    ret = HAL_HCD_RegisterCallback(&h_hcd_fs, HAL_HCD_CONNECT_CB_ID, hcd_connect_callback);
    if (ret != HAL_OK)
    {
        return hal_status_to_usbh_status(ret);
    }

    ret = HAL_HCD_RegisterCallback(&h_hcd_fs, HAL_HCD_DISCONNECT_CB_ID, hcd_disconnect_callback);
    if (ret != HAL_OK)
    {
        return hal_status_to_usbh_status(ret);
    }

    ret = HAL_HCD_RegisterCallback(&h_hcd_fs, HAL_HCD_PORT_ENABLED_CB_ID, hcd_port_enabled_callback);
    if (ret != HAL_OK)
    {
        return hal_status_to_usbh_status(ret);
    }

    ret = HAL_HCD_RegisterCallback(&h_hcd_fs, HAL_HCD_PORT_DISABLED_CB_ID, hcd_port_disabled_callback);
    if (ret != HAL_OK)
    {
        return hal_status_to_usbh_status(ret);
    }

    ret = HAL_HCD_RegisterHC_NotifyURBChangeCallback(&h_hcd_fs, hcd_hc_notify_urb_change_callback);
    if (ret != HAL_OK)
    {
        return hal_status_to_usbh_status(ret);
    }

    USBH_LL_SetTimer(phost, HAL_HCD_GetCurrentFrame(&h_hcd_fs));

    return USBH_OK;
}

/**
  * @brief  De-Initialize the low level portion of the host driver.
  * @param  phost: Host handle
  * @retval USBH status
  */
USBH_StatusTypeDef USBH_LL_DeInit(USBH_HandleTypeDef *phost)
{
    HAL_StatusTypeDef ret;
    ret = HAL_HCD_UnRegisterCallback(&h_hcd_fs, HAL_HCD_SOF_CB_ID);
    if (ret != HAL_OK)
    {
        return hal_status_to_usbh_status(ret);
    }

    ret = HAL_HCD_UnRegisterCallback(&h_hcd_fs, HAL_HCD_CONNECT_CB_ID);
    if (ret != HAL_OK)
    {
        return hal_status_to_usbh_status(ret);
    }

    ret = HAL_HCD_UnRegisterCallback(&h_hcd_fs, HAL_HCD_DISCONNECT_CB_ID);
    if (ret != HAL_OK)
    {
        return hal_status_to_usbh_status(ret);
    }

    ret = HAL_HCD_UnRegisterCallback(&h_hcd_fs, HAL_HCD_PORT_ENABLED_CB_ID);
    if (ret != HAL_OK)
    {
        return hal_status_to_usbh_status(ret);
    }

    ret = HAL_HCD_UnRegisterCallback(&h_hcd_fs, HAL_HCD_PORT_DISABLED_CB_ID);
    if (ret != HAL_OK)
    {
        return hal_status_to_usbh_status(ret);
    }

    ret = HAL_HCD_UnRegisterHC_NotifyURBChangeCallback(&h_hcd_fs);
    if (ret != HAL_OK)
    {
        return hal_status_to_usbh_status(ret);
    }

    ret = HAL_HCD_DeInit(&h_hcd_fs);
    if (ret != HAL_OK)
    {
        return hal_status_to_usbh_status(ret);
    }

    if (_error != HAL_OK)
    {
        return hal_status_to_usbh_status(_error);
    }

    ret = HAL_HCD_UnRegisterCallback(&h_hcd_fs, HAL_HCD_MSPINIT_CB_ID);
    if (ret != HAL_OK)
    {
        return hal_status_to_usbh_status(ret);
    }

    ret = HAL_HCD_UnRegisterCallback(&h_hcd_fs, HAL_HCD_MSPDEINIT_CB_ID);
    if (ret != HAL_OK)
    {
        return hal_status_to_usbh_status(ret);
    }

    return USBH_OK;
}

/**
  * @brief  Start the low level portion of the host driver.
  * @param  phost: Host handle
  * @retval USBH status
  */
USBH_StatusTypeDef USBH_LL_Start(USBH_HandleTypeDef *phost)
{
    HAL_StatusTypeDef hal_status = HAL_HCD_Start(phost->pData);

    return hal_status_to_usbh_status(hal_status);
}

/**
  * @brief  Stop the low level portion of the host driver.
  * @param  phost: Host handle
  * @retval USBH status
  */
USBH_StatusTypeDef USBH_LL_Stop(USBH_HandleTypeDef *phost)
{
    HAL_StatusTypeDef hal_status = HAL_HCD_Stop(phost->pData);

    return hal_status_to_usbh_status(hal_status);
}

/**
  * @brief  Return the USB host speed from the low level driver.
  * @param  phost: Host handle
  * @retval USBH speeds
  */
USBH_SpeedTypeDef USBH_LL_GetSpeed(USBH_HandleTypeDef *phost)
{
    switch (HAL_HCD_GetCurrentSpeed(phost->pData))
    {
        case 0 : return USBH_SPEED_HIGH;
        case 1 : return USBH_SPEED_FULL;
        case 2 : return USBH_SPEED_LOW;
        default: return USBH_SPEED_FULL;
    }

    return USBH_SPEED_FULL;
}

/**
  * @brief  Reset the Host port of the low level driver.
  * @param  phost: Host handle
  * @retval USBH status
  */
USBH_StatusTypeDef USBH_LL_ResetPort(USBH_HandleTypeDef *phost)
{
    HAL_StatusTypeDef hal_status = HAL_HCD_ResetPort(phost->pData);

    return hal_status_to_usbh_status(hal_status);
}

/**
  * @brief  Return the last transferred packet size.
  * @param  phost: Host handle
  * @param  pipe: Pipe index
  * @retval Packet size
  */
uint32_t USBH_LL_GetLastXferSize(USBH_HandleTypeDef *phost, uint8_t pipe)
{
    return HAL_HCD_HC_GetXferCount(phost->pData, pipe);
}

/**
  * @brief  Open a pipe of the low level driver.
  * @param  phost: Host handle
  * @param  pipe_num: Pipe index
  * @param  epnum: Endpoint number
  * @param  dev_address: Device USB address
  * @param  speed: Device Speed
  * @param  ep_type: Endpoint type
  * @param  mps: Endpoint max packet size
  * @retval USBH status
  */
USBH_StatusTypeDef USBH_LL_OpenPipe(USBH_HandleTypeDef *phost, uint8_t pipe_num, uint8_t epnum,
                                    uint8_t dev_address, uint8_t speed, uint8_t ep_type, uint16_t mps)
{
    HAL_StatusTypeDef hal_status = HAL_HCD_HC_Init(phost->pData, pipe_num, epnum,
                               dev_address, speed, ep_type, mps);

    return hal_status_to_usbh_status(hal_status);
}

/**
  * @brief  Close a pipe of the low level driver.
  * @param  phost: Host handle
  * @param  pipe: Pipe index
  * @retval USBH status
  */
USBH_StatusTypeDef USBH_LL_ClosePipe(USBH_HandleTypeDef *phost, uint8_t pipe)
{
    HAL_StatusTypeDef hal_status = HAL_HCD_HC_Halt(phost->pData, pipe);

    return hal_status_to_usbh_status(hal_status);
}

/**
  * @brief  Submit a new URB to the low level driver.
  * @param  phost: Host handle
  * @param  pipe: Pipe index
  *         This parameter can be a value from 1 to 15
  * @param  direction : Channel number
  *          This parameter can be one of the these values:
  *           0 : Output
  *           1 : Input
  * @param  ep_type : Endpoint Type
  *          This parameter can be one of the these values:
  *            @arg EP_TYPE_CTRL: Control type
  *            @arg EP_TYPE_ISOC: Isochrounous type
  *            @arg EP_TYPE_BULK: Bulk type
  *            @arg EP_TYPE_INTR: Interrupt type
  * @param  token : Endpoint Type
  *          This parameter can be one of the these values:
  *            @arg 0: PID_SETUP
  *            @arg 1: PID_DATA
  * @param  pbuff : pointer to URB data
  * @param  length : Length of URB data
  * @param  do_ping : activate do ping protocol (for high speed only)
  *          This parameter can be one of the these values:
  *           0 : do ping inactive
  *           1 : do ping active
  * @retval Status
  */
USBH_StatusTypeDef USBH_LL_SubmitURB(USBH_HandleTypeDef *phost, uint8_t pipe, uint8_t direction,
                                     uint8_t ep_type, uint8_t token, uint8_t *pbuff, uint16_t length,
                                     uint8_t do_ping)
{
    HAL_StatusTypeDef hal_status = HAL_HCD_HC_SubmitRequest(phost->pData, pipe, direction ,
                                        ep_type, token, pbuff, length,
                                        do_ping);
    return hal_status_to_usbh_status(hal_status);
}

/**
  * @brief  Get a URB state from the low level driver.
  * @param  phost: Host handle
  * @param  pipe: Pipe index
  *         This parameter can be a value from 1 to 15
  * @retval URB state
  *          This parameter can be one of the these values:
  *            @arg URB_IDLE
  *            @arg URB_DONE
  *            @arg URB_NOTREADY
  *            @arg URB_NYET
  *            @arg URB_ERROR
  *            @arg URB_STALL
  */
USBH_URBStateTypeDef USBH_LL_GetURBState(USBH_HandleTypeDef *phost, uint8_t pipe)
{
    return (USBH_URBStateTypeDef)HAL_HCD_HC_GetURBState(phost->pData, pipe);
}

/**
  * @brief  Drive VBUS.
  * @param  phost: Host handle
  * @param  state : VBUS state
  *          This parameter can be one of the these values:
  *           0 : VBUS Inactive
  *           1 : VBUS Active
  * @retval Status
  */
USBH_StatusTypeDef USBH_LL_DriverVBUS(USBH_HandleTypeDef *phost, uint8_t state)
{
    if (state == 0)
    {
        usb_host_powerswitch_disable();
    }
    else
    {
        usb_host_powerswitch_enable();
    }

    USBH_Delay(200);

    return USBH_OK;
}

/**
  * @brief  Set toggle for a pipe.
  * @param  phost: Host handle
  * @param  pipe: Pipe index
  * @param  toggle: toggle (0/1)
  * @retval Status
  */
USBH_StatusTypeDef USBH_LL_SetToggle(USBH_HandleTypeDef *phost, uint8_t pipe, uint8_t toggle)
{
    HCD_HandleTypeDef *pHandle = phost->pData;

    if (pHandle->hc[pipe].ep_is_in)
    {
        pHandle->hc[pipe].toggle_in = toggle;
    }
    else
    {
        pHandle->hc[pipe].toggle_out = toggle;
    }

    return USBH_OK;
}

/**
  * @brief  Return the current toggle of a pipe.
  * @param  phost: Host handle
  * @param  pipe: Pipe index
  * @retval toggle (0/1)
  */
uint8_t USBH_LL_GetToggle(USBH_HandleTypeDef *phost, uint8_t pipe)
{
    HCD_HandleTypeDef *pHandle = phost->pData;
    uint8_t toggle = 0;

    if (pHandle->hc[pipe].ep_is_in)
    {
        toggle = pHandle->hc[pipe].toggle_in;
    }
    else
    {
        toggle = pHandle->hc[pipe].toggle_out;
    }

    return toggle;
}

/**
  * @brief  Delay routine for the USB Host Library
  * @param  Delay: Delay in ms
  * @retval None
  */
void USBH_Delay(uint32_t Delay)
{
    vTaskDelay(pdMS_TO_TICKS(Delay));
}

/**
  * @brief  Returns the USB status depending on the HAL status:
  * @param  hal_status: HAL status
  * @retval USB status
  */
static USBH_StatusTypeDef hal_status_to_usbh_status(const HAL_StatusTypeDef hal_status)
{
    switch (hal_status)
    {
        case HAL_OK      : return USBH_OK;
        case HAL_ERROR   : return USBH_FAIL;
        case HAL_BUSY    : return USBH_BUSY;
        case HAL_TIMEOUT : return USBH_FAIL;
        default          : return USBH_FAIL;
    }

    return USBH_FAIL;
}

