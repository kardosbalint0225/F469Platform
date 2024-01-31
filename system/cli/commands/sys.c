/*
 * sys.c
 *
 *  Created on: 2023. dec. 12.
 *      Author: Balint
 */
#include "embedded_cli.h"
#include "stm32f4xx_hal.h"

#include <stdio.h>

typedef union
{
    struct
    {
        uint32_t b0 : 8;
        uint32_t b1 : 8;
        uint32_t b2 : 8;
        uint32_t b3 : 8;
    };
    uint32_t w;
} hal_uid_t;

const uint32_t hal_sysclk_source[4] = {
    RCC_SYSCLKSOURCE_HSI, RCC_SYSCLKSOURCE_HSE, RCC_SYSCLKSOURCE_PLLCLK, RCC_SYSCLKSOURCE_PLLRCLK
};

const uint32_t hal_ahbclk_div[9] = {
    RCC_SYSCLK_DIV1, RCC_SYSCLK_DIV2, RCC_SYSCLK_DIV4, RCC_SYSCLK_DIV8,
    RCC_SYSCLK_DIV16, RCC_SYSCLK_DIV64, RCC_SYSCLK_DIV128, RCC_SYSCLK_DIV256,
    RCC_SYSCLK_DIV512
};

const uint32_t hal_apbclk_div[5] = {
    RCC_HCLK_DIV1, RCC_HCLK_DIV2, RCC_HCLK_DIV4, RCC_HCLK_DIV8, RCC_HCLK_DIV16
};

const char * sysclk_source[4] = {
    "HSI", "HSE", "PLLCLK", "PLLRCLK"
};

const char * ahbclk_div[9] = {
    "/1", "/2", "/4", "/8", "/16", "/64", "/128", "/256", "/512"
};

const char * apbclk_div[5] = {
    "/1", "/2", "/4", "/8", "/16"
};

const uint32_t hal_rtc_clksrc[34] = {
    RCC_RTCCLKSOURCE_NO_CLK, RCC_RTCCLKSOURCE_LSE, RCC_RTCCLKSOURCE_LSI, RCC_RTCCLKSOURCE_HSE_DIVX,
    RCC_RTCCLKSOURCE_HSE_DIV2, RCC_RTCCLKSOURCE_HSE_DIV3, RCC_RTCCLKSOURCE_HSE_DIV4,
    RCC_RTCCLKSOURCE_HSE_DIV5, RCC_RTCCLKSOURCE_HSE_DIV6, RCC_RTCCLKSOURCE_HSE_DIV7,
    RCC_RTCCLKSOURCE_HSE_DIV8, RCC_RTCCLKSOURCE_HSE_DIV9, RCC_RTCCLKSOURCE_HSE_DIV10,
    RCC_RTCCLKSOURCE_HSE_DIV11, RCC_RTCCLKSOURCE_HSE_DIV12, RCC_RTCCLKSOURCE_HSE_DIV13,
    RCC_RTCCLKSOURCE_HSE_DIV14, RCC_RTCCLKSOURCE_HSE_DIV15, RCC_RTCCLKSOURCE_HSE_DIV16,
    RCC_RTCCLKSOURCE_HSE_DIV17, RCC_RTCCLKSOURCE_HSE_DIV18, RCC_RTCCLKSOURCE_HSE_DIV19,
    RCC_RTCCLKSOURCE_HSE_DIV20, RCC_RTCCLKSOURCE_HSE_DIV21, RCC_RTCCLKSOURCE_HSE_DIV22,
    RCC_RTCCLKSOURCE_HSE_DIV23, RCC_RTCCLKSOURCE_HSE_DIV24, RCC_RTCCLKSOURCE_HSE_DIV25,
    RCC_RTCCLKSOURCE_HSE_DIV26, RCC_RTCCLKSOURCE_HSE_DIV27, RCC_RTCCLKSOURCE_HSE_DIV28,
    RCC_RTCCLKSOURCE_HSE_DIV29, RCC_RTCCLKSOURCE_HSE_DIV30, RCC_RTCCLKSOURCE_HSE_DIV31,
};

const uint32_t hal_regulator_vos[3] = {
    PWR_REGULATOR_VOLTAGE_SCALE1, PWR_REGULATOR_VOLTAGE_SCALE2, PWR_REGULATOR_VOLTAGE_SCALE3
};

const char * regulator_vos[3] = {
    "Scale 1", "Scale 2", "Scale 3"
};

const char * rtc_clksrc[34] = {
    "NO CLK", "LSE", "LSI", "HSE /x", "HSE /2", "HSE /3", "HSE /4", "HSE /5", "HSE /6", "HSE /7",
    "HSE /8", "HSE /9", "HSE /10", "HSE /11", "HSE /12", "HSE /13", "HSE /14", "HSE /15", "HSE /16",
    "HSE /17", "HSE /18", "HSE /19", "HSE /20", "HSE /21", "HSE /22", "HSE /23", "HSE /24", "HSE /25",
    "HSE /26", "HSE /27", "HSE /28", "HSE /29", "HSE /30", "HSE /31",
};

static uint32_t find(const uint32_t *array, const uint32_t size, const uint32_t value);

/**
 * @brief  Function that is executed when the sysinfo command is entered.
 *         Displays system related informations (versions, clock states)
 *
 * @param  cli (not used)
 * @param  args (not used)
 * @param  context (not used)
 *
 * @retval None
 */
void cli_command_sysinfo(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)args;
    (void)context;

    uint32_t revid = HAL_GetREVID();
    uint32_t devid = HAL_GetDEVID();

    hal_uid_t uidw0 = {
        .w = HAL_GetUIDw0(),
    };

    hal_uid_t uidw1 = {
        .w = HAL_GetUIDw1(),
    };

    hal_uid_t uidw2 = {
        .w = HAL_GetUIDw2(),
    };

    uint32_t sysclk = HAL_RCC_GetSysClockFreq();
    uint32_t hclk = HAL_RCC_GetHCLKFreq();
    uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();
    uint32_t pclk2 = HAL_RCC_GetPCLK2Freq();

    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    HAL_RCC_GetOscConfig(&RCC_OscInitStruct);

    uint32_t pFLatency;
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    HAL_RCC_GetClockConfig(&RCC_ClkInitStruct, &pFLatency);

    printf("    MCU Revision ID         : 0x%04lx\r\n", revid);
    printf("    MCU Device ID           : 0x%3lx\r\n", devid);
    printf("    MCU UID                 : %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\r\n",
                                          uidw2.b3, uidw2.b2, uidw2.b1, uidw2.b0,
                                          uidw1.b3, uidw1.b2, uidw1.b1, uidw1.b0,
                                          uidw0.b3, uidw0.b2, uidw0.b1, uidw0.b0);
    printf("    HSE State               : %s\r\n", (RCC_HSE_ON == RCC_OscInitStruct.HSEState) ? ("On") : ((RCC_HSE_OFF == RCC_OscInitStruct.HSEState) ? ("Off") : ("Bypass")));
    printf("    LSE State               : %s\r\n", (RCC_LSE_ON == RCC_OscInitStruct.LSEState) ? ("On") : ((RCC_LSE_OFF == RCC_OscInitStruct.LSEState) ? ("Off") : ("Bypass")));
    printf("    HSI State               : %s\r\n", (RCC_HSI_ON == RCC_OscInitStruct.HSIState) ? ("On") : ("Off"));
    printf("    LSI State               : %s\r\n", (RCC_LSI_ON == RCC_OscInitStruct.LSIState) ? ("On") : ("Off"));
    printf("    PLL State               : %s\r\n", (RCC_PLL_ON == RCC_OscInitStruct.PLL.PLLState) ? ("On") : ((RCC_PLL_OFF == RCC_OscInitStruct.PLL.PLLState) ? ("Off") : ("None")));
    printf("    PLL Source              : %s\r\n", (RCC_PLLSOURCE_HSE == RCC_OscInitStruct.PLL.PLLSource) ? ("HSE") : ("HSI"));
    printf("    PLL                     : M = /%lu, N = x%lu, P = /%lu, Q = /%lu\r\n",
                                          RCC_OscInitStruct.PLL.PLLM, RCC_OscInitStruct.PLL.PLLN,
                                          RCC_OscInitStruct.PLL.PLLP, RCC_OscInitStruct.PLL.PLLQ);

    uint32_t sysclk_source_idx = find(hal_sysclk_source, (uint32_t)(sizeof(hal_sysclk_source) / sizeof(uint32_t)), RCC_ClkInitStruct.SYSCLKSource);
    assert(0xFFFFFFFFUL != sysclk_source_idx);
    assert(sysclk_source_idx < sizeof(sysclk_source) / sizeof(char *));

    uint32_t ahbclk_div_idx = find(hal_ahbclk_div, (uint32_t)(sizeof(hal_ahbclk_div) / sizeof(uint32_t)), RCC_ClkInitStruct.AHBCLKDivider);
    assert(0xFFFFFFFFUL != ahbclk_div_idx);
    assert(ahbclk_div_idx < sizeof(ahbclk_div) / sizeof(char *));

    uint32_t apb1clk_div_idx = find(hal_apbclk_div, (uint32_t)(sizeof(hal_apbclk_div) / sizeof(uint32_t)), RCC_ClkInitStruct.APB1CLKDivider);
    assert(0xFFFFFFFFUL != apb1clk_div_idx);
    assert(apb1clk_div_idx < sizeof(apbclk_div) / sizeof(char *));

    uint32_t apb2clk_div_idx = find(hal_apbclk_div, (uint32_t)(sizeof(hal_apbclk_div) / sizeof(uint32_t)), RCC_ClkInitStruct.APB2CLKDivider);
    assert(0xFFFFFFFFUL != apb2clk_div_idx);
    assert(apb2clk_div_idx < sizeof(apbclk_div) / sizeof(char *));

    RCC_PeriphCLKInitTypeDef RCC_PeriphClkInit = {0};
    HAL_RCCEx_GetPeriphCLKConfig(&RCC_PeriphClkInit);

    uint32_t rtc_clksrc_idx = find(hal_rtc_clksrc, (uint32_t)(sizeof(hal_rtc_clksrc) / sizeof(uint32_t)), RCC_PeriphClkInit.RTCClockSelection);
    assert(0xFFFFFFFFUL != rtc_clksrc_idx);
    assert(rtc_clksrc_idx < sizeof(rtc_clksrc) / sizeof(char *));

    printf("    SYSCLK Source           : %s\r\n", sysclk_source[sysclk_source_idx]);
    printf("    SYSCLK Frequency        : %lu Hz\r\n", sysclk);
    printf("    AHB Prescaler           : %s\r\n", ahbclk_div[ahbclk_div_idx]);
    printf("    HCLK Frequency          : %lu Hz\r\n", hclk);
    printf("    APB1 Prescaler          : %s\r\n", apbclk_div[apb1clk_div_idx]);
    printf("    APB1 (PCLK1) Frequency  : %lu Hz\r\n", pclk1);
    printf("    APB2 Prescaler          : %s\r\n", apbclk_div[apb2clk_div_idx]);
    printf("    APB2 (PCLK2) Frequency  : %lu Hz\r\n", pclk2);
    printf("    SysTick Clock Source    : %s\r\n", (SYSTICK_CLKSOURCE_HCLK == (SYSTICK_CLKSOURCE_HCLK & SysTick->CTRL)) ? ("HCLK") : ("HCLK /8"));
    printf("    RTC Clock source        : %s\r\n", rtc_clksrc[rtc_clksrc_idx]);
    printf("    PLLI2S                  : N = x%lu, R = /%lu\r\n", RCC_PeriphClkInit.PLLI2S.PLLI2SN, RCC_PeriphClkInit.PLLI2S.PLLI2SR);

    uint32_t vos = HAL_PWREx_GetVoltageRange();
    uint32_t regulator_vos_idx = find(hal_regulator_vos, (uint32_t)(sizeof(hal_regulator_vos) / sizeof(uint32_t)), vos);
    assert(0xFFFFFFFFUL != regulator_vos_idx);
    assert(regulator_vos_idx < sizeof(regulator_vos) / sizeof(char *));

    printf("    Regulator voltage scale : %s\r\n", regulator_vos[regulator_vos_idx]);
}

/**
 * @brief  Finds the index of a value in the given array
 *
 * @param  array the array to be checked
 * @param  size  the size of the array
 * @param  value the value to be found
 *
 * @retval returns the index of the value,
 *         0xFFFFFFFFul if the value can not be found
 */
static uint32_t find(const uint32_t *array, const uint32_t size, const uint32_t value)
{
    uint32_t index = 0xFFFFFFFFul;

    for (uint32_t i = 0; i < size; i++)
    {
        if (array[i] == value)
        {
            index = i;
        }
    }

    return index;
}

