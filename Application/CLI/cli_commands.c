/*
 * cli_commands.c
 *
 *  Created on: 2023. jul. 15.
 *      Author: Balint
 */
#include "cli_commands.h"
#include "cli_config.h"
#include "cli.h"

#include "stm32f4xx_hal.h"

#include "rtc.h"
#include "stdio_base.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

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

static void cli_command_runtime_stats(EmbeddedCli *cli, char *args, void *context);
static void cli_command_task_stats(EmbeddedCli *cli, char *args, void *context);
static void cli_command_get_date(EmbeddedCli *cli, char *args, void *context);
static void cli_command_get_time(EmbeddedCli *cli, char *args, void *context);
static void cli_command_set_date(EmbeddedCli *cli, char *args, void *context);
static void cli_command_set_time(EmbeddedCli *cli, char *args, void *context);
static void cli_command_sysinfo(EmbeddedCli *cli, char *args, void *context);

static bool is_number(const char s);
static bool is_time_command_string_valid(const char *time_string, const uint32_t len);
static bool is_date_command_string_valid(const char *date_string, const uint32_t len);
static void convert_string_to_time(uint8_t *hour, uint8_t *min, uint8_t *sec, const char *time_string);
static void convert_string_to_date(uint8_t *day, uint8_t *month, uint32_t *year, const char *date_string);
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
    stdio_write((char *)clear_string, sizeof(clear_string));
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

    const char * const header = "Task            Abs Time        %% Time"
                                "\r\n****************************************\r\n";
    int len = snprintf(cli_output_buffer, sizeof(cli_output_buffer), header);
    assert_param(len > 0);
    vTaskGetRunTimeStats(cli_output_buffer + len);

    len = strnlen(cli_output_buffer, sizeof(cli_output_buffer));
    stdio_write(cli_output_buffer, len);
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

    const char * const header = "Task          State  Priority   Stack   #"
                                "\r\n************************************************\r\n";
    int len = snprintf(cli_output_buffer, sizeof(cli_output_buffer), header);
    assert_param(len > 0);
    vTaskList(cli_output_buffer + len);

    len = strnlen(cli_output_buffer, sizeof(cli_output_buffer));
    stdio_write(cli_output_buffer, len);
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

    uint8_t day = 0;
    uint8_t month = 0;
    uint32_t year = 0;
//    uint8_t weekday;
    //rtc_get_date(&day, &month, &year, &weekday);
    assert_param(0);
    int len = snprintf(cli_output_buffer,
                       sizeof(cli_output_buffer),
                       "\r\n    %04lu.%02d.%02d.\r\n",
                       year,
                       month,
                       day);
    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);
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

    uint8_t hours = 0;
    uint8_t minutes = 0;
    uint8_t seconds = 0;
    //uint32_t subseconds;
    //rtc_get_time(&hours, &minutes, &seconds, &subseconds);
    assert_param(0);

    int len = snprintf(cli_output_buffer,
                       sizeof(cli_output_buffer),
                       "\r\n    %02d:%02d:%02d\r\n",
                       hours,
                       minutes,
                       seconds);
    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);
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

    int len;

    if (NULL == args)
    {
        len = snprintf(cli_output_buffer,
                       sizeof(cli_output_buffer),
                       "\r\n    Invalid command argument\r\n");
        assert_param(len > 0);
        stdio_write(cli_output_buffer, len);
        return;
    }

    embeddedCliTokenizeArgs(args);

    const char *date_to_set = embeddedCliGetToken(args, 1);
    assert_param(date_to_set);

    const uint32_t param_len = strnlen(date_to_set, CLI_CMD_BUFFER_SIZE);

    if (true == is_date_command_string_valid(date_to_set, param_len))
    {
        uint8_t day;
        uint8_t month;
        uint32_t year;
//        uint8_t weekday;

        convert_string_to_date(&day, &month, &year, date_to_set);

//        rtc_set_date(day, month, year);
//        rtc_get_date(&day, &month, &year, &weekday);
        assert_param(0);

        len = snprintf(cli_output_buffer,
                       sizeof(cli_output_buffer),
                       "\r\n    Date set to: %04lu.%02d.%02d.\r\n",
                       year, month, day);
    }
    else
    {
        len = snprintf(cli_output_buffer,
                       sizeof(cli_output_buffer),
                       "\r\n    Invalid parameter\r\n");
    }

    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);
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

    int len;

    if (NULL == args)
    {
        len = snprintf(cli_output_buffer,
                       sizeof(cli_output_buffer),
                       "\r\n    Invalid command argument\r\n");
        assert_param(len > 0);
        stdio_write(cli_output_buffer, len);
        return;
    }

    embeddedCliTokenizeArgs(args);

    const char *time_to_set = embeddedCliGetToken(args, 1);
    assert_param(time_to_set);

    const uint32_t param_len = strnlen(time_to_set, CLI_CMD_BUFFER_SIZE);

    if (true == is_time_command_string_valid(time_to_set, param_len))
    {
        uint8_t hours;
        uint8_t minutes;
        uint8_t seconds;
//        uint32_t subseconds;

        convert_string_to_time(&hours, &minutes, &seconds, time_to_set);

//        rtc_set_time(hours, minutes, seconds);
//        rtc_get_time(&hours, &minutes, &seconds, &subseconds);
        assert_param(0);

        len = snprintf(cli_output_buffer,
                       sizeof(cli_output_buffer),
                       "\r\n    Time set to: %02d:%02d:%02d\r\n",
                       hours, minutes, seconds);
    }
    else
    {
        len = snprintf(cli_output_buffer,
                       sizeof(cli_output_buffer),
                       "\r\n    Invalid parameter\r\n");
    }

    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);
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
    assert_param(time_string);

    bool retv = true;
    if (len < 8UL)
    {
        retv = false;
    }

    if (time_string[2] != ':' || time_string[5] != ':')
    {
        retv = false;
    }

    uint32_t non_number_chars = 0UL;
    for (uint32_t i = 0; i < len; i++)
    {
        if (true != is_number(time_string[i]))
        {
            non_number_chars = non_number_chars + 1;
        }
    }

    if (2UL != non_number_chars)
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
    assert_param(date_string);

    bool retv = true;
    if (len < 8UL)
    {
        retv = false;
    }

    if (date_string[4] != '.' || date_string[7] != '.' || date_string[10] != '.')
    {
        retv = false;
    }

    uint32_t non_number_chars = 0UL;
    for (uint32_t i = 0; i < len; i++)
    {
        if (true != is_number(date_string[i]))
        {
            non_number_chars = non_number_chars + 1;
        }
    }

    if (3UL != non_number_chars)
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
static void convert_string_to_time(uint8_t *hour, uint8_t *min, uint8_t *sec, const char *time_string)
{
    assert_param(hour);
    assert_param(min);
    assert_param(sec);
    assert_param(time_string);

    *hour = (uint8_t)((time_string[0] - '0') * 10) + (uint8_t)(time_string[1] - '0');
    *min = (uint8_t)((time_string[3] - '0') * 10) + (uint8_t)(time_string[4] - '0');
    *sec = (uint8_t)((time_string[6] - '0') * 10) + (uint8_t)(time_string[7] - '0');
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
static void convert_string_to_date(uint8_t *day, uint8_t *month, uint32_t *year, const char *date_string)
{
    assert_param(day);
    assert_param(month);
    assert_param(year);
    assert_param(date_string);

    *year = (uint32_t)((date_string[0] - '0') * 1000) +
            (uint32_t)((date_string[1] - '0') * 100) +
            (uint32_t)((date_string[2] - '0') * 10) +
            (uint32_t)((date_string[3] - '0') * 1);
    *month = (uint8_t)((date_string[5] - '0') * 10) + (uint8_t)(date_string[6] - '0');
    *day = (uint8_t)((date_string[8] - '0') * 10) + (uint8_t)(date_string[9] - '0');
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

    int len;

    len = snprintf(cli_output_buffer,
                   sizeof(cli_output_buffer),
                   "    FreeRTOS Kernel Version : %s\r\n",
                   FREERTOS_KERNEL_VERSION);
    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);

    len = snprintf(cli_output_buffer,
                   sizeof(cli_output_buffer),
                   "    HAL Driver Version      : %s\r\n",
                   STM32F4XX_HAL_DRIVER_VERSION);
    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);

    len = snprintf(cli_output_buffer,
                   sizeof(cli_output_buffer),
                   "    Embedded CLI Version    : %s\r\n",
                   EMBEDDED_CLI_VERSION);
    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);

    len = snprintf(cli_output_buffer,
                   sizeof(cli_output_buffer),
                   "    CMSIS GCC Version       : %s\r\n",
                   CMSIS_GCC_VERSION);
    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);

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

    len = snprintf(cli_output_buffer,
                   sizeof(cli_output_buffer),
                   "    MCU Revision ID         : 0x%04lx\r\n",
                   revid);
    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);

    len = snprintf(cli_output_buffer,
                   sizeof(cli_output_buffer),
                   "    MCU Device ID           : 0x%3lx\r\n",
                   devid);
    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);

    len = snprintf(cli_output_buffer,
                   sizeof(cli_output_buffer),
                   "    MCU UID                 : %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\r\n",
                   uidw2.b3, uidw2.b2, uidw2.b1, uidw2.b0,
                   uidw1.b3, uidw1.b2, uidw1.b1, uidw1.b0,
                   uidw0.b3, uidw0.b2, uidw0.b1, uidw0.b0);
    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);

    len = snprintf(cli_output_buffer,
                   sizeof(cli_output_buffer),
                   "    HSE State               : %s\r\n",
                   (RCC_HSE_ON == RCC_OscInitStruct.HSEState) ? ("On") : ((RCC_HSE_OFF == RCC_OscInitStruct.HSEState) ? ("Off") : ("Bypass")));
    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);

    len = snprintf(cli_output_buffer,
                   sizeof(cli_output_buffer),
                   "    LSE State               : %s\r\n",
                   (RCC_LSE_ON == RCC_OscInitStruct.LSEState) ? ("On") : ((RCC_LSE_OFF == RCC_OscInitStruct.LSEState) ? ("Off") : ("Bypass")));
    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);

    len = snprintf(cli_output_buffer,
                   sizeof(cli_output_buffer),
                   "    HSI State               : %s\r\n",
                   (RCC_HSI_ON == RCC_OscInitStruct.HSIState) ? ("On") : ("Off"));
    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);

    len = snprintf(cli_output_buffer,
                   sizeof(cli_output_buffer),
                   "    LSI State               : %s\r\n",
                   (RCC_LSI_ON == RCC_OscInitStruct.LSIState) ? ("On") : ("Off"));
    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);

    len = snprintf(cli_output_buffer,
                   sizeof(cli_output_buffer),
                   "    PLL State               : %s\r\n",
                   (RCC_PLL_ON == RCC_OscInitStruct.PLL.PLLState) ? ("On") : ((RCC_PLL_OFF == RCC_OscInitStruct.PLL.PLLState) ? ("Off") : ("None")));
    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);

    len = snprintf(cli_output_buffer,
                   sizeof(cli_output_buffer),
                   "    PLL Source              : %s\r\n",
                   (RCC_PLLSOURCE_HSE == RCC_OscInitStruct.PLL.PLLSource) ? ("HSE") : ("HSI"));
    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);

    len = snprintf(cli_output_buffer,
                   sizeof(cli_output_buffer),
                   "    PLL                     : M = /%lu, N = x%lu, P = /%lu, Q = /%lu\r\n",
                   RCC_OscInitStruct.PLL.PLLM, RCC_OscInitStruct.PLL.PLLN,
                   RCC_OscInitStruct.PLL.PLLP, RCC_OscInitStruct.PLL.PLLQ);
    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);

    uint32_t sysclk_source_idx = find(hal_sysclk_source, (uint32_t)(sizeof(hal_sysclk_source) / sizeof(uint32_t)), RCC_ClkInitStruct.SYSCLKSource);
    assert_param(0xFFFFFFFFUL != sysclk_source_idx);
    assert_param(sysclk_source_idx < sizeof(sysclk_source) / sizeof(char *));

    uint32_t ahbclk_div_idx = find(hal_ahbclk_div, (uint32_t)(sizeof(hal_ahbclk_div) / sizeof(uint32_t)), RCC_ClkInitStruct.AHBCLKDivider);
    assert_param(0xFFFFFFFFUL != ahbclk_div_idx);
    assert_param(ahbclk_div_idx < sizeof(ahbclk_div) / sizeof(char *));

    uint32_t apb1clk_div_idx = find(hal_apbclk_div, (uint32_t)(sizeof(hal_apbclk_div) / sizeof(uint32_t)), RCC_ClkInitStruct.APB1CLKDivider);
    assert_param(0xFFFFFFFFUL != apb1clk_div_idx);
    assert_param(apb1clk_div_idx < sizeof(apbclk_div) / sizeof(char *));

    uint32_t apb2clk_div_idx = find(hal_apbclk_div, (uint32_t)(sizeof(hal_apbclk_div) / sizeof(uint32_t)), RCC_ClkInitStruct.APB2CLKDivider);
    assert_param(0xFFFFFFFFUL != apb2clk_div_idx);
    assert_param(apb2clk_div_idx < sizeof(apbclk_div) / sizeof(char *));

    RCC_PeriphCLKInitTypeDef RCC_PeriphClkInit = {0};
    HAL_RCCEx_GetPeriphCLKConfig(&RCC_PeriphClkInit);

    uint32_t rtc_clksrc_idx = find(hal_rtc_clksrc, (uint32_t)(sizeof(hal_rtc_clksrc) / sizeof(uint32_t)), RCC_PeriphClkInit.RTCClockSelection);
    assert_param(0xFFFFFFFFUL != rtc_clksrc_idx);
    assert_param(rtc_clksrc_idx < sizeof(rtc_clksrc) / sizeof(char *));

    len = snprintf(cli_output_buffer,
                   sizeof(cli_output_buffer),
                   "    SYSCLK Source           : %s\r\n",
                   sysclk_source[sysclk_source_idx]);
    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);

    len = snprintf(cli_output_buffer,
                   sizeof(cli_output_buffer),
                   "    SYSCLK Frequency        : %lu Hz\r\n",
                   sysclk);
    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);

    len = snprintf(cli_output_buffer,
                   sizeof(cli_output_buffer),
                   "    AHB Prescaler           : %s\r\n",
                   ahbclk_div[ahbclk_div_idx]);
    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);

    len = snprintf(cli_output_buffer,
                   sizeof(cli_output_buffer),
                   "    HCLK Frequency          : %lu Hz\r\n",
                   hclk);
    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);

    len = snprintf(cli_output_buffer,
                   sizeof(cli_output_buffer),
                   "    APB1 Prescaler          : %s\r\n",
                   apbclk_div[apb1clk_div_idx]);
    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);

    len = snprintf(cli_output_buffer,
                   sizeof(cli_output_buffer),
                   "    APB1 (PCLK1) Frequency  : %lu Hz\r\n",
                   pclk1);
    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);

    len = snprintf(cli_output_buffer,
                   sizeof(cli_output_buffer),
                   "    APB2 Prescaler          : %s\r\n",
                   apbclk_div[apb2clk_div_idx]);
    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);

    len = snprintf(cli_output_buffer,
                   sizeof(cli_output_buffer),
                   "    APB2 (PCLK2) Frequency  : %lu Hz\r\n",
                   pclk2);
    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);

    len = snprintf(cli_output_buffer,
                   sizeof(cli_output_buffer),
                   "    SysTick Clock Source    : %s\r\n",
                   (SYSTICK_CLKSOURCE_HCLK == (SYSTICK_CLKSOURCE_HCLK & SysTick->CTRL)) ? ("HCLK") : ("HCLK /8"));
    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);

    len = snprintf(cli_output_buffer,
                   sizeof(cli_output_buffer),
                   "    RTC Clock source        : %s\r\n",
                   rtc_clksrc[rtc_clksrc_idx]);
    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);

    len = snprintf(cli_output_buffer,
                   sizeof(cli_output_buffer),
                   "    PLLI2S                  : N = x%lu, R = /%lu\r\n",
                   RCC_PeriphClkInit.PLLI2S.PLLI2SN, RCC_PeriphClkInit.PLLI2S.PLLI2SR);
    assert_param(len > 0);
    stdio_write(cli_output_buffer, len);
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

