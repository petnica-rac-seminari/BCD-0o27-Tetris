/**
 * @file        ch405_labs_commands.c
 * @author      Espressif Systems (original), Florian Schütz (fschuetz@ieee.org)
 * @brief       Driver and pattern generations functions for driving addressable
 *              leds.
 * @version     0.9
 * @date        2023-07-27
 * @copyright   Modifications copyright (c) 2023 Florian Schütz, released under MIT license
 * @copyright   Orignal code copyright (c) 2016-2021 Espressif Systems (Shanghai) CO LTD, released under Apache-2.0 license
 * 
 * This code is an adaption of esp_console.c by Espressif Systems (Shanghai)
 * CO LTD. It implements the same functionality. The derivation is needed for 
 * the implementation of extended console features implemented in 
 * ch405labs_esp_console.hpp to avoid naming conflicts.
 * 
 * Original code by Espressif Systems:
 *  SPDX-FileCopyrightText: 2016-2021 Espressif Systems (Shanghai) CO LTD
 *  SPDX-License-Identifier: Apache-2.0
 * 
 * Modifications from original code:
 *  - Names of functions to avoid clash with orginial function names when 
 *      including esp-idf components.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#include "esp_log.h"
#include "ch405_labs_esp_console.h"
#include "esp_system.h"
//#include "ch405_labs_linenoise/ch405_labs_linenoise.h"
#include "ch405labs_linenoise.h"
#include "argtable3/argtable3.h"
#include "sys/queue.h"

#define ANSI_COLOR_DEFAULT      39      /** Default foreground color */

typedef struct ch405_labs_cmd_item_ {
    /**
     * Command name (statically allocated by application)
     */
    const char *command;
    /**
     * Help text (statically allocated by application), may be NULL.
     */
    const char *help;
    /**
     * Hint text, usually lists possible arguments, dynamically allocated.
     * May be NULL.
     */
    char *hint;
    ch405_labs_esp_console_cmd_func_t func;    //!< pointer to the command handler
    void *argtable;                 //!< optional pointer to arg table
    SLIST_ENTRY(ch405_labs_cmd_item_) next;    //!< next command in the list
} ch405_labs_cmd_item_t;

/** linked list of command structures */
static SLIST_HEAD(cmd_list_, ch405_labs_cmd_item_) s_cmd_list;

/** run-time configuration options */
static ch405_labs_esp_console_config_t s_config;

/** temporary buffer used for command line parsing */
static char *s_tmp_line_buf;

static const ch405_labs_cmd_item_t *find_command_by_name(const char *name);

esp_err_t ch405_labs_esp_console_init(const ch405_labs_esp_console_config_t *config) {
    if (!config) {
        return ESP_ERR_INVALID_ARG;
    }
    if (s_tmp_line_buf) {
        return ESP_ERR_INVALID_STATE;
    }
    memcpy(&s_config, config, sizeof(s_config));
    if (s_config.hint_color == 0) {
        s_config.hint_color = ANSI_COLOR_DEFAULT;
    }
    s_tmp_line_buf = calloc(config->max_cmdline_length, 1);
    if (s_tmp_line_buf == NULL) {
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}

esp_err_t ch405_labs_esp_console_deinit(void)
{
    if (!s_tmp_line_buf) {
        return ESP_ERR_INVALID_STATE;
    }
    free(s_tmp_line_buf);
    s_tmp_line_buf = NULL;
    ch405_labs_cmd_item_t *it, *tmp;
    SLIST_FOREACH_SAFE(it, &s_cmd_list, next, tmp) {
        SLIST_REMOVE(&s_cmd_list, it, ch405_labs_cmd_item_, next);
        free(it->hint);
        free(it);
    }
    return ESP_OK;
}

esp_err_t ch405_labs_esp_console_cmd_register(const ch405_labs_esp_console_cmd_t *cmd)
{
    ch405_labs_cmd_item_t *item = NULL;
    if (!cmd || cmd->command == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    if (strchr(cmd->command, ' ') != NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    item = (ch405_labs_cmd_item_t *)find_command_by_name(cmd->command);
    if (!item) {
        // not registered before
        item = calloc(1, sizeof(*item));
        if (item == NULL) {
            return ESP_ERR_NO_MEM;
        }
    } else {
        // remove from list and free the old hint, because we will alloc new hint for the command
        SLIST_REMOVE(&s_cmd_list, item, ch405_labs_cmd_item_, next);
        free(item->hint);
    }
    item->command = cmd->command;
    item->help = cmd->help;
    if (cmd->hint) {
        /* Prepend a space before the hint. It separates command name and
         * the hint. arg_print_syntax below adds this space as well.
         */
        int unused __attribute__((unused));
        unused = asprintf(&item->hint, " %s", cmd->hint);
    } else if (cmd->argtable) {
        /* Generate hint based on cmd->argtable */
        char *buf = NULL;
        size_t buf_size = 0;
        FILE *f = open_memstream(&buf, &buf_size);
        if (f != NULL) {
            arg_print_syntax(f, cmd->argtable, NULL);
            fclose(f);
        }
        item->hint = buf;
    }
    item->argtable = cmd->argtable;
    item->func = cmd->func;
    ch405_labs_cmd_item_t *last = SLIST_FIRST(&s_cmd_list);
    if (last == NULL) {
        SLIST_INSERT_HEAD(&s_cmd_list, item, next);
    } else {
        ch405_labs_cmd_item_t *it;
        while ((it = SLIST_NEXT(last, next)) != NULL) {
            last = it;
        }
        SLIST_INSERT_AFTER(last, item, next);
    }
    return ESP_OK;
}

void ch405_labs_esp_console_get_completion(const char *buf, ch405_labs_linenoiseCompletions *lc)
{
    size_t len = strlen(buf);
    if (len == 0) {
        return;
    }
    ch405_labs_cmd_item_t *it;
    SLIST_FOREACH(it, &s_cmd_list, next) {
        /* Check if command starts with buf */
        if (strncmp(buf, it->command, len) == 0) {
            ch405_labs_linenoiseAddCompletion(lc, it->command);
        }
    }
}

const char *ch405_labs_esp_console_get_hint(const char *buf, int *color, int *bold)
{
    size_t len = strlen(buf);
    ch405_labs_cmd_item_t *it;
    SLIST_FOREACH(it, &s_cmd_list, next) {
        if (strlen(it->command) == len &&
                strncmp(buf, it->command, len) == 0) {
            *color = s_config.hint_color;
            *bold = s_config.hint_bold;
            return it->hint;
        }
    }
    return NULL;
}

static const ch405_labs_cmd_item_t *find_command_by_name(const char *name)
{
    const ch405_labs_cmd_item_t *cmd = NULL;
    ch405_labs_cmd_item_t *it;
    size_t len = strlen(name);
    SLIST_FOREACH(it, &s_cmd_list, next) {
        if (strlen(it->command) == len &&
                strcmp(name, it->command) == 0) {
            cmd = it;
            break;
        }
    }
    return cmd;
}

esp_err_t ch405_labs_esp_console_run(const char *cmdline, int *cmd_ret)
{
    if (s_tmp_line_buf == NULL) {
        return ESP_ERR_INVALID_STATE;
    }
    char **argv = (char **) calloc(s_config.max_cmdline_args, sizeof(char *));
    if (argv == NULL) {
        return ESP_ERR_NO_MEM;
    }
    strlcpy(s_tmp_line_buf, cmdline, s_config.max_cmdline_length);

    size_t argc = ch405_labs_esp_console_split_argv(s_tmp_line_buf, argv,
                                         s_config.max_cmdline_args);
    if (argc == 0) {
        free(argv);
        return ESP_ERR_INVALID_ARG;
    }
    const ch405_labs_cmd_item_t *cmd = find_command_by_name(argv[0]);
    if (cmd == NULL) {
        free(argv);
        return ESP_ERR_NOT_FOUND;
    }
    *cmd_ret = (*cmd->func)(argc, argv);
    free(argv);
    return ESP_OK;
}

static int ch405_labs_help_command(int argc, char **argv)
{
    ch405_labs_cmd_item_t *it;

    /* Print summary of each command */
    SLIST_FOREACH(it, &s_cmd_list, next) {
        if (it->help == NULL) {
            continue;
        }
        /* First line: command name and hint
         * Pad all the hints to the same column
         */
        const char *hint = (it->hint) ? it->hint : "";
        printf("%-s %s\n", it->command, hint);
        /* Second line: print help.
         * Argtable has a nice helper function for this which does line
         * wrapping.
         */
        printf("  "); // arg_print_formatted does not indent the first line
        arg_print_formatted(stdout, 2, 78, it->help);
        /* Finally, print the list of arguments */
        if (it->argtable) {
            arg_print_glossary(stdout, (void **) it->argtable, "  %12s  %s\n");
        }
        printf("\n");
    }
    return 0;
}

esp_err_t ch405_labs_esp_console_register_help_command(void)
{
    ch405_labs_esp_console_cmd_t command = {
        .command = "help",
        .help = "Print the list of registered commands",
        .func = &ch405_labs_help_command
    };
    return ch405_labs_esp_console_cmd_register(&command);
}
