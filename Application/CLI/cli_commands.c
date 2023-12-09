/*
 * cli_commands.c
 *
 *  Created on: 2023. jul. 15.
 *      Author: Balint
 */
#include "cli_commands.h"
#include "cli_config.h"
#include "cli.h"

#include "macros/units.h"

#include "stm32f4xx_hal.h"
#include "rtc.h"
#include "vfs.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

#include "library_versions.h"

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

const char * rtc_clksrc[34] = {
    "NO CLK", "LSE", "LSI", "HSE /x", "HSE /2", "HSE /3", "HSE /4", "HSE /5", "HSE /6", "HSE /7",
    "HSE /8", "HSE /9", "HSE /10", "HSE /11", "HSE /12", "HSE /13", "HSE /14", "HSE /15", "HSE /16",
    "HSE /17", "HSE /18", "HSE /19", "HSE /20", "HSE /21", "HSE /22", "HSE /23", "HSE /24", "HSE /25",
    "HSE /26", "HSE /27", "HSE /28", "HSE /29", "HSE /30", "HSE /31",
};

static char cli_output_buffer[CLI_PRINT_BUFFER_SIZE];
static char current_directory[255] = {'\0'};

static void cli_command_runtime_stats(EmbeddedCli *cli, char *args, void *context);
static void cli_command_task_stats(EmbeddedCli *cli, char *args, void *context);
static void cli_command_get_date(EmbeddedCli *cli, char *args, void *context);
static void cli_command_get_time(EmbeddedCli *cli, char *args, void *context);
static void cli_command_set_date(EmbeddedCli *cli, char *args, void *context);
static void cli_command_set_time(EmbeddedCli *cli, char *args, void *context);
static void cli_command_sysinfo(EmbeddedCli *cli, char *args, void *context);
static void cli_command_ls(EmbeddedCli *cli, char *args, void *context);

static bool is_number(const char s);
static bool is_time_command_string_valid(const char *time_string, const uint32_t len);
static bool is_date_command_string_valid(const char *date_string, const uint32_t len);
static void convert_string_to_time(int *hour, int *min, int *sec, const char *time_string);
static void convert_string_to_date(int *day, int *month, int *year, const char *date_string);
static uint32_t find(const uint32_t *array, const uint32_t size, const uint32_t value);

static CliCommandBinding clear_binding = {
    .name = "clear",
    .help = "Clears the console",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_clear_terminal
};

static CliCommandBinding runtime_stats_binding = {
    .name = "runtimestats",
    .help = "Displays how much processing time each FreeRTOS task has used",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_runtime_stats
};

static CliCommandBinding task_stats_binding = {
    .name = "taskstats",
    .help = "Displays a table showing the state of each FreeRTOS task",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_task_stats
};

static CliCommandBinding get_date_binding = {
    .name = "date",
    .help = "Displays the date in yyyy.mm.dd. format",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_get_date
};

static CliCommandBinding get_time_binding = {
    .name = "time",
    .help = "Displays the time in hh:mm:ss format",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_get_time
};

static CliCommandBinding set_date_binding = {
    .name = "setdate",
    .help = "setdate <yyyy.mm.dd.>: Sets the current date",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_set_date
};

static CliCommandBinding set_time_binding = {
    .name = "settime",
    .help = "settime <hh:mm:ss>: Sets the current time",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_set_time
};

static CliCommandBinding sysinfo_binding = {
    .name = "sysinfo",
    .help = "Displays system related informations",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_sysinfo
};

static CliCommandBinding ls_binding = {
    .name = "ls",
    .help = "Lists all files and folders in the current working directory",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_ls
};

/**
 * @brief  Adds the command bindings to the cli instance
 * @param  None
 * @retval None
 * @note   -
 */
void cli_init_command_bindings(void)
{
    EmbeddedCli *cli = cli_get_pointer();
    embeddedCliAddBinding(cli, clear_binding);
    embeddedCliAddBinding(cli, runtime_stats_binding);
    embeddedCliAddBinding(cli, task_stats_binding);
    embeddedCliAddBinding(cli, get_date_binding);
    embeddedCliAddBinding(cli, get_time_binding);
    embeddedCliAddBinding(cli, set_date_binding);
    embeddedCliAddBinding(cli, set_time_binding);
    embeddedCliAddBinding(cli, sysinfo_binding);
    embeddedCliAddBinding(cli, ls_binding);
}

/**
 * @brief  Clears the terminal
 * @param  cli (not used)
 * @param  args (not used)
 * @param  context (not used)
 * @retval None
 * @note   -
 */
void cli_command_clear_terminal(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)args;
    (void)context;

    const char * const clear_string = "\33[2J";
    printf("%s", clear_string);
}

/**
 * @brief  Function that is executed when runtimestats command entered
 *         Displays the absolute and relative times of each task
 * @param  cli (not used)
 * @param  args (not used)
 * @param  context (not used)
 * @retval None
 * @note   -
 */
static void cli_command_runtime_stats(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)args;
    (void)context;

    printf("Task            Abs Time        %% Time\r\n");
    printf("****************************************\r\n");

    vTaskGetRunTimeStats(cli_output_buffer);

    printf("%s\r\n", cli_output_buffer);
}

/**
 * @brief  Function that is executed when taskstats command entered
 *         Displays the state, priority and stack usage of each task
 * @param  cli (not used)
 * @param  args (not used)
 * @param  context (not used)
 * @retval None
 * @note   -
 */
static void cli_command_task_stats(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)args;
    (void)context;

    printf("Task          State  Priority   Stack   #\r\n");
    printf("************************************************\r\n");

    vTaskList(cli_output_buffer);

    printf("%s\r\n", cli_output_buffer);
}

/**
 * @brief  Function that is executed when date command entered
 *         Displays the current date set to the RTC
 * @param  cli (not used)
 * @param  args (not used)
 * @param  context (not used)
 * @retval None
 * @note   -
 */
static void cli_command_get_date(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)args;
    (void)context;

    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(cli_output_buffer, sizeof(cli_output_buffer), "%F", timeinfo);
    printf("    %s\r\n", cli_output_buffer);
}

/**
 * @brief  Function that is executed when time command entered
 *         Displays the current time set to the RTC
 * @param  cli (not used)
 * @param  args (not used)
 * @param  context (not used)
 * @retval None
 * @note   -
 */
static void cli_command_get_time(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)args;
    (void)context;

    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    strftime(cli_output_buffer, sizeof(cli_output_buffer), "%T", timeinfo);
    printf("    %s\r\n", cli_output_buffer);
}

/**
 * @brief  Function that is executed when setdate command entered
 *         Sets the current date to the RTC
 * @param  cli (not used)
 * @param  args command argument holding the date to be set
 * @param  context (not used)
 * @retval None
 * @note   -
 */
static void cli_command_set_date(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)context;

    if (NULL == args)
    {
        printf("\r\n    Invalid command argument\r\n");
        return;
    }

    embeddedCliTokenizeArgs(args);

    const char *date_to_set = embeddedCliGetToken(args, 1);
    assert(date_to_set);

    const uint32_t param_len = strnlen(date_to_set, CLI_CMD_BUFFER_SIZE);

    if (true != is_date_command_string_valid(date_to_set, param_len))
    {
        printf("\r\n    Invalid parameter\r\n");
        return;
    }

    struct tm t;
    int ret;
    ret = rtc_get_time(&t);
    if (0 != ret)
    {
        printf("\r\n    An error occurred while trying to access the RTC module. Error : %d\r\n", ret);
        return;
    }

    convert_string_to_date(&t.tm_mday, &t.tm_mon, &t.tm_year, date_to_set);

    ret = rtc_set_time(&t);
    if (0 != ret)
    {
        printf("\r\n    An error occurred while trying to access the RTC module. Error : %d\r\n", ret);
        return;
    }

    printf("\r\n    Date set to: %04d.%02d.%02d.\r\n", t.tm_year, t.tm_mon, t.tm_mday);
}

/**
 * @brief  Function that is executed when settime command entered
 *         Sets the current time to the RTC
 * @param  cli (not used)
 * @param  args command argument holding the time to be set
 * @param  context (not used)
 * @retval None
 * @note   -
 */
static void cli_command_set_time(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)context;

    if (NULL == args)
    {
        printf("\r\n    Invalid command argument\r\n");
        return;
    }

    embeddedCliTokenizeArgs(args);

    const char *time_to_set = embeddedCliGetToken(args, 1);
    assert(time_to_set);

    const uint32_t param_len = strnlen(time_to_set, CLI_CMD_BUFFER_SIZE);

    if (true != is_time_command_string_valid(time_to_set, param_len))
    {
        printf("\r\n    Invalid parameter\r\n");
        return;
    }

    struct tm t;
    int ret;

    ret = rtc_get_time(&t);
    if (0 != ret)
    {
        printf("\r\n    An error occurred while trying to access the RTC module. Error : %d\r\n", ret);
        return;
    }

    convert_string_to_time(&t.tm_hour, &t.tm_min, &t.tm_sec, time_to_set);

    ret = rtc_set_time(&t);
    if (0 != ret)
    {
        printf("\r\n    An error occurred while trying to access the RTC module. Error : %d\r\n", ret);
        return;
    }

    printf("\r\n    Time set to: %02d:%02d:%02d\r\n", t.tm_hour, t.tm_min, t.tm_sec);
}

/**
 * @brief   Checks whether the given input is an ASCII number or not
 * @param   s the input character
 * @retval  true if the argument is a number
 * @retval  false otherwise
 * @note    -
 */
static bool is_number(const char s)
{
    return ((s >= '0' && s <= '9') ? (true) : (false));
}

/**
 * @brief  Checks whether the settime command string is valid or not
 *
 * @param  time_string the string containing the argument of the settime command
 * @param  len the length of the time_string
 * @retval true if the time command string is valid (hh:mm:ss format)
 * @retval false otherwise
 * @note   -
 */
static bool is_time_command_string_valid(const char *time_string, const uint32_t len)
{
    assert(time_string);

    bool retv = true;
    if (len < 8)
    {
        retv = false;
    }

    if (time_string[2] != ':' || time_string[5] != ':')
    {
        retv = false;
    }

    uint32_t non_number_chars = 0;
    for (uint32_t i = 0; i < len; i++)
    {
        if (true != is_number(time_string[i]))
        {
            non_number_chars = non_number_chars + 1;
        }
    }

    if (2 != non_number_chars)
    {
        retv = false;
    }

    return retv;
}

/**
 * @brief  Checks whether the setdate command string is valid or not
 *
 * @param  date_string the string containing the argument of the setdate command
 * @param  len the length of the date_string
 * @retval true if the date command string is valid (yyyy.mm.dd. format)
 * @retval false otherwise
 */
static bool is_date_command_string_valid(const char *date_string, const uint32_t len)
{
    assert(date_string);

    bool retv = true;
    if (len < 8)
    {
        retv = false;
    }

    if (date_string[4] != '.' || date_string[7] != '.' || date_string[10] != '.')
    {
        retv = false;
    }

    uint32_t non_number_chars = 0;
    for (uint32_t i = 0; i < len; i++)
    {
        if (true != is_number(date_string[i]))
        {
            non_number_chars = non_number_chars + 1;
        }
    }

    if (3 != non_number_chars)
    {
        retv = false;
    }

    return retv;
}

/**
 * @brief  Extracts time information from a given string
 *
 * @param  hour where the hour value can be stored
 * @param  min  where the min value can be stored
 * @param  sec  where the sec value can be stored
 * @param  time_string the string to be extracted
 * @retval None
 * @note   -
 */
static void convert_string_to_time(int *hour, int *min, int *sec, const char *time_string)
{
    assert(hour);
    assert(min);
    assert(sec);
    assert(time_string);

    *hour = (int)((time_string[0] - '0') * 10) + (int)(time_string[1] - '0');
    *min = (int)((time_string[3] - '0') * 10) + (int)(time_string[4] - '0');
    *sec = (int)((time_string[6] - '0') * 10) + (int)(time_string[7] - '0');
}

/**
 * @brief  Extracts date information from a given string
 *
 * @param  day   where the day value can be stored
 * @param  month where the month value can be stored
 * @param  year  where the year value can be stored
 * @param  date_string the string to be extracted
 * @retval None
 * @note   -
 */
static void convert_string_to_date(int *day, int *month, int *year, const char *date_string)
{
    assert(day);
    assert(month);
    assert(year);
    assert(date_string);

    *year = (int)((date_string[0] - '0') * 1000) +
            (int)((date_string[1] - '0') * 100) +
            (int)((date_string[2] - '0') * 10) +
            (int)((date_string[3] - '0') * 1);
    *month = (int)((date_string[5] - '0') * 10) + (int)(date_string[6] - '0');
    *day = (int)((date_string[8] - '0') * 10) + (int)(date_string[9] - '0');
}

/**
 * @brief  Function that is executed when the sysinfo command is entered.
 *         Displays system related informations (versions, clock states)
 * @param  cli (not used)
 * @param  args (not used)
 * @param  context (not used)
 * @retval None
 * @note   -
 */
static void cli_command_sysinfo(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)args;
    (void)context;

    printf("    FreeRTOS Kernel Version : %s\r\n", FREERTOS_KERNEL_VERSION);
    printf("    HAL Driver Version      : %s\r\n", STM32F4XX_HAL_DRIVER_VERSION);
    printf("    Embedded CLI Version    : %s\r\n", EMBEDDED_CLI_VERSION);
    printf("    CMSIS GCC Version       : %s\r\n", CMSIS_GCC_VERSION);

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
}

static void _print_size(uint64_t size)
{
    unsigned long len;
    const char *unit;

    if (size == 0) {
        len = 0;
        unit = NULL;
    } else if ((size & (GiB(1) - 1)) == 0) {
        len = size / GiB(1);
        unit = "GiB";
    }
    else if ((size & (MiB(1) - 1)) == 0) {
        len = size / MiB(1);
        unit = "MiB";
    }
    else if ((size & (KiB(1) - 1)) == 0) {
        len = size / KiB(1);
        unit = "KiB";
    } else {
        len = size;
        unit = NULL;
    }

    if (unit) {
        printf("%8lu %s ", len, unit);
    } else {
        printf("%10lu B ", len);
    }
}

static void _print_df(vfs_DIR *dir)
{
    struct statvfs buf;
    int res = vfs_dstatvfs(dir, &buf);
    printf("%-16s ", dir->mp->mount_point);
    if (res < 0) {
        printf("statvfs failed: %s\n", strerror(-res));
        return;
    }

    _print_size(buf.f_blocks * buf.f_bsize);
    _print_size((buf.f_blocks - buf.f_bfree) * buf.f_bsize);
    _print_size(buf.f_bavail * buf.f_bsize);
    printf("%7lu%%\n", (unsigned long)(((buf.f_blocks - buf.f_bfree) * 100) / buf.f_blocks));
}

static void cli_command_ls(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)context;

    if (NULL != args)
    {
        embeddedCliTokenizeArgs(args);
        const char *path = embeddedCliGetToken(args, 1);
    }
    else
    {
        if (0 == strnlen(current_directory, sizeof(current_directory)))
        {
            puts("Mountpoint              Total         Used    Available     Use%");
            /* Iterate through all mount points */
            vfs_DIR it = { 0 };
            while (vfs_iterate_mount_dirs(&it)) {
                _print_df(&it);
            }
        }
    }

}

/**
 * @brief  Finds the index of a value in the given array
 *
 * @param  array the array to be checked
 * @param  size  the size of the array
 * @param  value the value to be found
 * @retval returns the index of the value,
 *         0xFFFFFFFFUL if the value can not be found
 */
static uint32_t find(const uint32_t *array, const uint32_t size, const uint32_t value)
{
    uint32_t index = 0xFFFFFFFFUL;

    for (uint32_t i = 0; i < size; i++)
    {
        if (array[i] == value)
        {
            index = i;
        }
    }

    return index;
}

