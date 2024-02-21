/*
 * rtc.c
 *
 *  Created on: 2023. dec. 12.
 *      Author: Balint
 */
#include "embedded_cli.h"
#include "cli_config.h"
#include "rtc.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>

static char _buffer[CLI_PRINT_BUFFER_SIZE];

static bool is_time_command_string_valid(const char *time_string, const uint32_t len);
static bool is_date_command_string_valid(const char *date_string, const uint32_t len);
static void convert_string_to_time(int *hour, int *min, int *sec, const char *time_string);
static void convert_string_to_date(int *day, int *month, int *year, const char *date_string);

/**
 * @brief  Function that is executed when date command entered
 *         Displays the current date set to the RTC
 *
 * @param  cli (not used)
 * @param  args (not used)
 * @param  context (not used)
 *
 * @retval None
 */
void cli_command_get_date(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)args;
    (void)context;

    time_t rawtime;
    struct tm timeinfo;

    time(&rawtime);
    localtime_r(&rawtime, &timeinfo);

    strftime(_buffer, sizeof(_buffer), "%F", &timeinfo);
    printf("    %s\r\n", _buffer);
}

/**
 * @brief  Function that is executed when time command entered
 *         Displays the current time set to the RTC
 *
 * @param  cli (not used)
 * @param  args (not used)
 * @param  context (not used)
 *
 * @retval None
 */
void cli_command_get_time(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)context;

    const char *ms_arg = embeddedCliGetToken(args, 1);

    struct timeval tv;
    struct tm timeinfo;

    gettimeofday(&tv, NULL);
    const uint32_t ms = (uint32_t)(tv.tv_usec / 1000);
    localtime_r(&tv.tv_sec, &timeinfo);

    strftime(_buffer, sizeof(_buffer), "%T", &timeinfo);

    if (0 == strncmp(ms_arg, "-ms", CLI_CMD_BUFFER_SIZE))
    {
        printf("    %s:%03lu\r\n", _buffer, ms);
    }
    else
    {
        printf("    %s\r\n", _buffer);
    }
}

/**
 * @brief  Function that is executed when setdate command entered
 *         Sets the current date to the RTC
 *
 * @param  cli (not used)
 * @param  args command argument holding the date to be set
 * @param  context (not used)
 *
 * @retval None
 */
void cli_command_set_date(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)context;

    if (NULL == args)
    {
        printf("\r\n    Invalid command argument\r\n");
        return;
    }

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

    int mday;
    int mon;
    int year;
    convert_string_to_date(&mday, &mon, &year, date_to_set);
    t.tm_year = year - 1900;
    t.tm_mon = mon - 1;
    t.tm_mday = mday;
    t.tm_isdst = -1;

    ret = rtc_set_time(&t);
    if (0 != ret)
    {
        printf("\r\n    An error occurred while trying to access the RTC module. Error : %d\r\n", ret);
        return;
    }

    printf("\r\n    Date set to: %04d.%02d.%02d.\r\n", year, mon, mday);
}

/**
 * @brief  Function that is executed when settime command entered
 *         Sets the current time to the RTC
 *
 * @param  cli (not used)
 * @param  args command argument holding the time to be set
 * @param  context (not used)
 *
 * @retval None
 */
void cli_command_set_time(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)context;

    if (NULL == args)
    {
        printf("\r\n    Invalid command argument\r\n");
        return;
    }

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
 * @brief  Checks whether the settime command string is valid or not
 *
 * @param  time_string the string containing the argument of the settime command
 * @param  len the length of the time_string
 *
 * @retval true if the time command string is valid (hh:mm:ss format)
 * @retval false otherwise
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
        if (0 == isdigit((unsigned char)time_string[i]))
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
 *
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
        if (0 == isdigit((unsigned char)date_string[i]))
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
 *
 * @retval None
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
 *
 * @retval None
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

