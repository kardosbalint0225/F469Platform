/*
 * misc.c
 *
 *  Created on: Feb 22, 2024
 *      Author: Balint
 */
#include "embedded_cli.h"
#include "FreeRTOS.h"
#include "task.h"
#include <assert.h>
#include <stdio.h>

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

void cli_command_stdintest(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)args;
    (void)context;

    TaskHandle_t h;
    h = xTaskGetHandle("stdin1test");
    xTaskNotify(h, 0, eIncrement);
    h = xTaskGetHandle("stdin2test");
    xTaskNotify(h, 0, eIncrement);
    h = xTaskGetHandle("stdin3test");
    xTaskNotify(h, 0, eIncrement);
    h = xTaskGetHandle("stdin4test");
    xTaskNotify(h, 0, eIncrement);
}

void cli_command_malloctest(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)args;
    (void)context;

    pvPortMalloc(0xFFFFFFFFul);
}

void cli_command_hardfaulttest(EmbeddedCli *cli, char *args, void *context)
{
    (void)cli;
    (void)args;
    (void)context;

    uint64_t i = *(uint64_t *)(0x20CDCDCD);
    printf("%llu", i);
}

