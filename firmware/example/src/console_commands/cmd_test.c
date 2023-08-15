/**
 * @file cmd_test.c
 * @author your name (you@domain.com)
 * @brief Just some test commands to get familiar with the system
 * @version 0.1
 * @date 2022-08-13
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "cmd_test.h"

static void register_add(void);
static void register_echo(void);

/** register all commands in this library */
void register_test(void)
{
    register_add();
    register_echo();
}


/** 'restart' command restarts the program */
static int add(int argc, char **argv)
{
    ESP_LOGI(TAG_COMMAND_TEST, "Reading number");
    int augend, addend;
    printf("Enter first summand:  ");
    scanf("%d", &augend);
    printf("\n"); 
    printf("Enter second summand: ");
    scanf("%d", &addend);
    printf("\n"); 
    
    int result = addend + augend;
    if(result == 666) {
        printf("%d + %d = the number of the beast\n", augend, addend);
    } else {
        printf("%d + %d = %d\n", augend, addend, result);
    }
    
    return 0;
}

static void register_add(void)
{
    const ch405_labs_esp_console_cmd_t cmd = {
        .command = "add",
        .help = "Reads a number from stdin.",
        .hint = NULL,
        .func = &add,
    };
    ESP_ERROR_CHECK( ch405_labs_esp_console_cmd_register(&cmd) );
}

/** 'restart' command restarts the program */
static int echo(int argc, char **argv)
{
    ESP_LOGI(TAG_COMMAND_TEST, "Echo back user input");
    char *userInput = malloc(100 * sizeof(char));

    printf("Enter something: ");
    fgets(userInput, 100, stdin);

    printf("Entered: %s\n", userInput);

    free(userInput);

    return 0;
}

static void register_echo(void)
{
    const ch405_labs_esp_console_cmd_t cmd = {
        .command = "echo",
        .help = "Reads user input from stdin and echos it back.",
        .hint = NULL,
        .func = &echo,
    };
    ESP_ERROR_CHECK( ch405_labs_esp_console_cmd_register(&cmd) );
}