/*
 * MIT License
 *
 * Copyright (c) 2024 Balint Kardos
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/**
 * @ingroup     system_cli
 * @{
 * @file        rtc.c
 * @brief       Real-Time Clock (RTC) Commands
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
 * @brief Function that is executed when the date command is entered
 *        Displays the current date set to the RTC
 *
 * This function retrieves the current system time, converts it to
 * a human-readable format representing the date, and prints it to the console.
 *
 * @param cli     Pointer to the EmbeddedCli instance (unused).
 * @param args    Pointer to the arguments passed to the command (unused).
 * @param context Pointer to additional context data (unused).
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
 * @brief Function that is executed when the time command is entered
 *        Displays the current time set to the RTC
 *
 * This function retrieves the current system time, converts it to a human-readable
 * format representing the time, and optionally prints milliseconds if specified
 * by the user.
 *
 * @param cli     Pointer to the EmbeddedCli instance (unused).
 * @param args    Pointer to the arguments passed to the command. It may contain "-ms" to specify milliseconds.
 * @param context Pointer to additional context data (unused).
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
 * @brief Function that is executed when the setdate command is entered
 *        Sets the current date to the RTC.
 *
 * This function parses the input argument representing the date to be set,
 * validates it, and sets the system date accordingly. The command argument
 * should be in the format "YYYY.MM.DD". If successful, it updates the
 * system date and prints a confirmation message.
 *
 * @param cli     Pointer to the EmbeddedCli instance (unused).
 * @param args    Pointer to the arguments passed to the command, containing the date to be set.
 * @param context Pointer to additional context data (unused).
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
 * @brief Function that is executed when the settime command is entered
 *        Sets the current time to the RTC.
 *
 * This function parses the input argument representing the time to be set,
 * validates it, and sets the system time accordingly. The command argument
 * should be in the format "HH:MM:SS". If successful, it updates the
 * system time and prints a confirmation message.
 *
 * @param cli     Pointer to the EmbeddedCli instance (unused).
 * @param args    Pointer to the arguments passed to the command, containing the time to be set.
 * @param context Pointer to additional context data (unused).
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
 * @brief Checks if a time string is in a valid format.
 *
 * This function checks whether the given time string is in a valid format
 * for setting the system time. The valid format should be "HH:MM:SS".
 *
 * @param  time_string Pointer to the time string to be validated.
 * @param  len         Length of the time string.
 * @return true if the time string is valid, false otherwise.
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
 * @brief Checks if a date string is in a valid format.
 *
 * This function checks whether the given date string is in a valid format
 * for setting the system date. The valid format should be "YYYY.MM.DD".
 *
 * @param  date_string Pointer to the date string to be validated.
 * @param  len         Length of the date string.
 * @return true if the date string is valid, false otherwise.
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
 * @brief Converts a time string to individual hour, minute, and second components.
 *
 * This function parses the given time string in the format "HH:MM:SS" and extracts
 * the hour, minute, and second components, storing them in the provided integer
 * pointers.
 *
 * @param hour        Pointer to an integer variable to store the extracted hour.
 * @param min         Pointer to an integer variable to store the extracted minute.
 * @param sec         Pointer to an integer variable to store the extracted second.
 * @param time_string Pointer to the time string to be parsed.
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
 * @brief Converts a date string to individual day, month, and year components.
 *
 * This function parses the given date string in the format "YYYY.MM.DD" and extracts
 * the year, month, and day components, storing them in the provided integer pointers.
 *
 * @param day         Pointer to an integer variable to store the extracted day.
 * @param month       Pointer to an integer variable to store the extracted month.
 * @param year        Pointer to an integer variable to store the extracted year.
 * @param date_string Pointer to the date string to be parsed.
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
/** @} */
