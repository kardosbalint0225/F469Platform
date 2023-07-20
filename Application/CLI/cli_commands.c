/*
 * commands.c
 *
 *  Created on: 2023. jul. 15.
 *      Author: Balint
 */
#include "embedded_cli.h"
#include "cli_commands.h"
#include "cli_config.h"
#include "cli.h"

#include "rtc.h"
#include "uart_console.h"

#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

static char cli_output_buffer[CLI_PRINT_BUFFER_SIZE];

static void cli_command_runtime_stats(EmbeddedCli *cli, char *args, void *context);
static void cli_command_task_stats(EmbeddedCli *cli, char *args, void *context);
static void cli_command_get_date(EmbeddedCli *cli, char *args, void *context);
static void cli_command_get_time(EmbeddedCli *cli, char *args, void *context);
static void cli_command_set_date(EmbeddedCli *cli, char *args, void *context);
static void cli_command_set_time(EmbeddedCli *cli, char *args, void *context);

static bool is_number(const char s);
static bool is_time_command_string_valid(const char *time_string, const uint32_t len);
static bool is_date_command_string_valid(const char *date_string, const uint32_t len);
static void convert_string_to_time(uint8_t *hour, uint8_t *min, uint8_t *sec, const char *time_string);
static void convert_string_to_date(uint8_t *day, uint8_t *month, uint8_t *year, const char *date_string);

static CliCommandBinding clear_binding = {
	.name = "clear",
	.help = "Clears the console",
	.tokenizeArgs = true,
	.context = NULL,
	.binding = cli_command_clear_terminal
};

static CliCommandBinding runtime_stats_binding = {
	.name = "runtime-stats",
	.help = "Displays how much processing time each FreeRTOS task has used",
	.tokenizeArgs = true,
	.context = NULL,
	.binding = cli_command_runtime_stats
};

static CliCommandBinding task_stats_binding = {
	.name = "task-stats",
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
	.name = "set-date",
	.help = "set-date <yyyy.mm.dd.>:\r\n Sets the current date",
	.tokenizeArgs = true,
	.context = NULL,
	.binding = cli_command_set_date
};

static CliCommandBinding set_time_binding = {
	.name = "set-time",
	.help = "set-time <hh:mm:ss>:\r\n Sets the current time",
	.tokenizeArgs = true,
	.context = NULL,
	.binding = cli_command_set_time
};

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
}

void cli_command_clear_terminal(EmbeddedCli *cli, char *args, void *context)
{
	(void)cli;
	(void)args;
	(void)context;

	const char * const clear_string = "\33[2J";
	__uart_console_write((char *)clear_string, sizeof(clear_string));
}

static void cli_command_runtime_stats(EmbeddedCli *cli, char *args, void *context)
{
	(void)cli;
	(void)args;
	(void)context;

	const char * const header = "Task            Abs Time        %% Time"
			                    "\r\n****************************************\r\n";
	int len = snprintf(cli_output_buffer, sizeof(cli_output_buffer), header);
	assert_param(len > 0);
	vTaskGetRunTimeStats( cli_output_buffer + len );

	len = strnlen(cli_output_buffer, sizeof(cli_output_buffer));
	__uart_console_write(cli_output_buffer, len);
}

static void cli_command_task_stats(EmbeddedCli *cli, char *args, void *context)
{
	(void)cli;
	(void)args;
	(void)context;

	const char * const header = "Task          State  Priority   Stack   #"
			                    "\r\n************************************************\r\n";
	int len = snprintf(cli_output_buffer, sizeof(cli_output_buffer), header);
	assert_param(len > 0);
	vTaskList( cli_output_buffer + len );

	len = strnlen(cli_output_buffer, sizeof(cli_output_buffer));
	__uart_console_write(cli_output_buffer, len);
}

static void cli_command_get_date(EmbeddedCli *cli, char *args, void *context)
{
	(void)cli;
	(void)args;
	(void)context;

	uint8_t day;
	uint8_t month;
	uint8_t year;
	uint8_t weekday;
	rtc_get_date(&day, &month, &year, &weekday);
	int len = snprintf(cli_output_buffer,
                       sizeof(cli_output_buffer),
                       "\r\n    %04d.%02d.%02d.\r\n",
                       (2000+year),
                       month,
                       day);
	assert_param(len > 0);
	__uart_console_write(cli_output_buffer, len);
}

static void cli_command_get_time(EmbeddedCli *cli, char *args, void *context)
{
	(void)cli;
	(void)args;
	(void)context;

	uint8_t hours;
	uint8_t minutes;
	uint8_t seconds;
	rtc_get_time(&hours, &minutes, &seconds);

	int len = snprintf(cli_output_buffer,
                       sizeof(cli_output_buffer),
                       "\r\n    %02d:%02d:%02d\r\n",
                       hours,
                       minutes,
                       seconds);
	assert_param(len > 0);
	__uart_console_write(cli_output_buffer, len);
}

static void cli_command_set_date(EmbeddedCli *cli, char *args, void *context)
{
	(void)cli;
	(void)context;

	int len;

	if (NULL == args) {
		len = snprintf(cli_output_buffer,
					   sizeof(cli_output_buffer),
                       "\r\n    Invalid command argument\r\n");
		assert_param(len > 0);
		__uart_console_write(cli_output_buffer, len);
		return;
	}

	embeddedCliTokenizeArgs(args);

	const char *date_to_set = embeddedCliGetToken(args, 1);
	assert_param(date_to_set);

	const uint32_t param_len = strnlen(date_to_set, CLI_CMD_BUFFER_SIZE);

	if (true == is_date_command_string_valid(date_to_set, param_len)) {
		uint8_t day;
		uint8_t month;
		uint8_t year;
		uint8_t weekday;

		convert_string_to_date(&day, &month, &year, date_to_set);

		rtc_set_date(day, month, year);
		rtc_get_date(&day, &month, &year, &weekday);

		len = snprintf(cli_output_buffer,
				       sizeof(cli_output_buffer),
					   "\r\n    Date set to: %04d.%02d.%02d.\r\n",
					   (2000+year), month, day);
	} else {
		len = snprintf(cli_output_buffer,
			           sizeof(cli_output_buffer),
					   "\r\n    Invalid parameter\r\n");
	}

	assert_param(len > 0);
	__uart_console_write(cli_output_buffer, len);
}

static void cli_command_set_time(EmbeddedCli *cli, char *args, void *context)
{
	(void)cli;
	(void)context;

	int len;

	if (NULL == args) {
		len = snprintf(cli_output_buffer,
					   sizeof(cli_output_buffer),
                       "\r\n    Invalid command argument\r\n");
		assert_param(len > 0);
		__uart_console_write(cli_output_buffer, len);
		return;
	}

	embeddedCliTokenizeArgs(args);

	const char *time_to_set = embeddedCliGetToken(args, 1);
	assert_param(time_to_set);

	const uint32_t param_len = strnlen(time_to_set, CLI_CMD_BUFFER_SIZE);

	if (true == is_time_command_string_valid(time_to_set, param_len)) {
		uint8_t hours;
		uint8_t minutes;
		uint8_t seconds;

		convert_string_to_time(&hours, &minutes, &seconds, time_to_set);

		rtc_set_time(hours, minutes, seconds);
		rtc_get_time(&hours, &minutes, &seconds);

		len = snprintf(cli_output_buffer,
					   sizeof(cli_output_buffer),
					   "\r\n    Time set to: %02d:%02d:%02d\r\n",
					   hours, minutes, seconds);
	} else {
		len = snprintf(cli_output_buffer,
					   sizeof(cli_output_buffer),
					   "\r\n    Invalid parameter\r\n");
	}

	assert_param(len > 0);
	__uart_console_write(cli_output_buffer, len);
}

/**
 * @brief   Checks whether the given input is an ASCII number or not
 * @param   s the input character
 * @retval  true if the argument is a number
 * @retval  false otherwise
 * @note    This is a helper function of the is_time_command_valid
 *          and is_date_command_valid
 */
static bool is_number(const char s)
{
	return ( (s >= '0' && s <= '9') ? (true) : (false) );
}

/**
 * @brief  Checks whether the set-time command string is valid or not
 *
 * @param  time_string the string containing the argument of the set-time command
 * @param  len the length of the time_string
 * @retval true if the time command string is valid (hh:mm:ss format)
 * @retval false otherwise
 * @note   This is a helper function of the set_time_command
 */
static bool is_time_command_string_valid(const char *time_string, const uint32_t len)
{
	assert_param(time_string);

	bool retv = true;
	if (len < 8UL) {
		retv = false;
	}

	if (time_string[2] != ':' || time_string[5] != ':') {
		retv = false;
	}

	uint32_t non_number_chars = 0UL;
	for (uint32_t i = 0; i < len; i++) {
		if (true != is_number(time_string[i])) {
			non_number_chars = non_number_chars + 1;
		}
	}

	if (2UL != non_number_chars) {
		retv = false;
	}

	return retv;
}

/**
 * @brief  Checks whether the set-date command string is valid or not
 *
 * @param  date_string the string containing the argument of the set-date command
 * @param  len the length of the date_string
 * @retval true if the date command string is valid (yyyy.mm.dd. format)
 * @retval false otherwise
 * @note   This is a helper function of the set_date_command
 */
static bool is_date_command_string_valid(const char *date_string, const uint32_t len)
{
	assert_param(date_string);

	bool retv = true;
	if (len < 8UL) {
		retv = false;
	}

	if (date_string[4] != '.' || date_string[7] != '.' || date_string[10] != '.') {
		retv = false;
	}

	uint32_t non_number_chars = 0UL;
	for (uint32_t i = 0; i < len; i++) {
		if (true != is_number(date_string[i])) {
			non_number_chars = non_number_chars + 1;
		}
	}

	if (3UL != non_number_chars) {
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
 * @note   This is a helper function of the set_time_command
 */
static void convert_string_to_time(uint8_t *hour, uint8_t *min, uint8_t *sec, const char *time_string)
{
	assert_param(hour);
	assert_param(min);
	assert_param(sec);
	assert_param(time_string);

	*hour = (uint8_t)((time_string[0] - '0')*10) + (uint8_t)(time_string[1] - '0');
	*min  = (uint8_t)((time_string[3] - '0')*10) + (uint8_t)(time_string[4] - '0');
	*sec  = (uint8_t)((time_string[6] - '0')*10) + (uint8_t)(time_string[7] - '0');
}

/**
 * @brief  Extracts date information from a given string
 *
 * @param  day   where the day value can be stored
 * @param  month where the month value can be stored
 * @param  year  where the year value can be stored
 * @param  date_string the string to be extracted
 * @retval None
 * @note   This is a helper function of the set_date_command
 */
static void convert_string_to_date(uint8_t *day, uint8_t *month, uint8_t *year, const char *date_string)
{
	assert_param(day);
	assert_param(month);
	assert_param(year);
	assert_param(date_string);

	*year  = (uint8_t)((date_string[2] - '0')*10) + (uint8_t)(date_string[3] - '0');
	*month = (uint8_t)((date_string[5] - '0')*10) + (uint8_t)(date_string[6] - '0');
	*day   = (uint8_t)((date_string[8] - '0')*10) + (uint8_t)(date_string[9] - '0');
}
