/*
 * cli_commands.c
 *
 *  Created on: 2023. jul. 15.
 *      Author: Balint
 */
#include "cli.h"
#include "cli_commands.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

extern void cli_command_runtime_stats(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_task_stats(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_sysinfo(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_get_date(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_get_time(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_set_date(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_set_time(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_version(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_memstat(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_ls(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_cd(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_cp(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_r(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_rm(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_mv(EmbeddedCli *cli, char *args, void *context);
extern void cli_command_mkdir(EmbeddedCli *cli, char *args, void *context);

void cli_command_assert(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)args;
    (void)context;

    printf(" 1.  0123456789ABCDEFGHIJKLMNOPQSTUVWXYZabcdefghijklmnopqrstuvwxyz\r\n");
    printf(" 2.  AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAaaaaaaaaaaaaaaaaaaaaaaaaaa\r\n");
    printf(" 3.  0001112222222222233ddddddddddddddDDDDDDDDDDDDDDDDDDDDDDDDDDDD\r\n");
    printf(" 4.  EEEEEEEEEHAAAAAAAAAAAAAAAAAAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\r\n");
    printf(" 5.  01234567CCCCCCCCCCCCCCCCCCCCccccccccccccccccccccccccccccccccc\r\n");
    printf(" 6.  abhbhdsfdsfhjkhkdjhfbsdfjhsdjjbnfdsbvdhfjdhfusdhfksjfdhsdvfdd\r\n");
    printf(" 7.  zurewrhdfhmfbdcvbusdhbfusjfuewhbdatfsdajhdvjhgfkjfdghskufhukh\r\n");
    printf(" 8.  01234567CCCCCCCCCCCCCCBbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\r\n");
    printf(" 9.  01RrrrrrrrrRRRRrrrrrrrrrrrRRRRRRRRrrrrrrrrrrrrrrrrrrrrrrrrrrr\r\n");
    printf(" 10. iuifdshfdsztrwezubcdsziuczfuzwkjfuihgdsfkjhdfdsiuzfukiefhskud\r\n");
    printf(" 11. 0123456789ABCDEFGHIJKLMNOPQSTUVWXYZabcdefghijklmnopqrstuvwxyz\r\n");
    printf(" 12. AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAaaaaaaaaaaaaaaaaaaaaaaaaaa\r\n");
    printf(" 13. 0001112222222222233ddddddddddddddDDDDDDDDDDDDDDDDDDDDDDDDDDDD\r\n");
    printf(" 14. EEEEEEEEEHAAAAAAAAAAAAAAAAAAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\r\n");
    printf(" 15. 01234567CCCCCCCCCCCCCCCCCCCCccccccccccccccccccccccccccccccccc\r\n");
    printf(" 16. abhbhdsfdsfhjkhkdjhfbsdfjhsdjjbnfdsbvdhfjdhfusdhfksjfdhsdvfdd\r\n");
    printf(" 17. zurewrhdfhmfbdcvbusdhbfusjfuewhbdatfsdajhdvjhgfkjfdghskufhukh\r\n");
    printf(" 18. 01234567CCCCCCCCCCCCCCBbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\r\n");
    printf(" 19. 01RrrrrrrrrRRRRrrrrrrrrrrrRRRRRRRRrrrrrrrrrrrrrrrrrrrrrrrrrrr\r\n");
    printf(" 20. iuifdshfdsztrwezubcdsziuczfuzwkjfuihgdsfkjhdfdsiuzfukiefhskud\r\n");
    printf(" 21. 0123456789ABCDEFGHIJKLMNOPQSTUVWXYZabcdefghijklmnopqrstuvwxyz\r\n");
    printf(" 22. AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAaaaaaaaaaaaaaaaaaaaaaaaaaa\r\n");
    printf(" 23. 0001112222222222233ddddddddddddddDDDDDDDDDDDDDDDDDDDDDDDDDDDD\r\n");
    printf(" 24. EEEEEEEEEHAAAAAAAAAAAAAAAAAAaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\r\n");
    printf(" 25. 01234567CCCCCCCCCCCCCCCCCCCCccccccccccccccccccccccccccccccccc\r\n");
    printf(" 26. abhbhdsfdsfhjkhkdjhfbsdfjhsdjjbnfdsbvdhfjdhfusdhfksjfdhsdvfdd\r\n");
    printf(" 27. zurewrhdfhmfbdcvbusdhbfusjfuewhbdatfsdajhdvjhgfkjfdghskufhukh\r\n");
    printf(" 28. 01234567CCCCCCCCCCCCCCBbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\r\n");
    printf(" 29. 01RrrrrrrrrRRRRrrrrrrrrrrrRRRRRRRRrrrrrrrrrrrrrrrrrrrrrrrrrrr\r\n");
    printf(" 30. iuifdshfdsztrwezubcdsziuczfuzwkjfuihgdsfkjhdfdsiuzfukiefhskud\r\n");
    assert(0);

}

static CliCommandBinding assert_binding = {
    .name = "assert",
    .help = "Assert",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_assert
};

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
    .help = "Displays the time in hh:mm:ss format. Optional -ms argument can be used to display the millisecond component",
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

static CliCommandBinding version_binding = {
    .name = "version",
    .help = "Displays the application version and the used library versions",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_version
};

static CliCommandBinding memstat_binding = {
    .name = "memstat",
    .help = "Displays the detailed state of the application and system memory",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_memstat
};

static CliCommandBinding ls_binding = {
    .name = "ls",
    .help = "Lists all files and folders in the current working directory",
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_ls
};

static CliCommandBinding cd_binding = {
    .name = "cd",
    .help = "todo",//TODO
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_cd
};

static CliCommandBinding cp_binding = {
    .name = "cp",
    .help = "todo",//TODO
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_cp
};

static CliCommandBinding r_binding = {
    .name = "r",
    .help = "todo",//TODO
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_r
};

static CliCommandBinding rm_binding = {
    .name = "rm",
    .help = "todo",//TODO
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_rm
};

static CliCommandBinding mv_binding = {
    .name = "mv",
    .help = "todo",//TODO
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_mv
};

static CliCommandBinding mkdir_binding = {
    .name = "mkdir",
    .help = "todo",//TODO
    .tokenizeArgs = true,
    .context = NULL,
    .binding = cli_command_mkdir
};

/**
 * @brief  Adds the command bindings to the cli instance
 *
 * @param  None
 *
 * @retval None
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
    embeddedCliAddBinding(cli, version_binding);
    embeddedCliAddBinding(cli, memstat_binding);
    embeddedCliAddBinding(cli, ls_binding);
    embeddedCliAddBinding(cli, cd_binding);
    embeddedCliAddBinding(cli, cp_binding);
    embeddedCliAddBinding(cli, r_binding);
    embeddedCliAddBinding(cli, rm_binding);
    embeddedCliAddBinding(cli, mv_binding);
    embeddedCliAddBinding(cli, mkdir_binding);

    embeddedCliAddBinding(cli, assert_binding);
}

/**
 * @brief  Clears the terminal
 *
 * @param  cli (not used)
 * @param  args (not used)
 * @param  context (not used)
 *
 * @retval None
 */
void cli_command_clear_terminal(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)args;
    (void)context;

    const char * const clear_string = "\33[2J";
    printf("%s", clear_string);
}








