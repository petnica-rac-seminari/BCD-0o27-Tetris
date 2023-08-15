/*
 * @file        split_argv.c
 * @author      Espressif Systems (original), Florian Schütz (fschuetz@ieee.org)
 * @brief       Driver and pattern generations functions for driving addressable
 *              leds.
 * @version     0.9
 * @date        2023-07-27
 * @copyright   Copyright (c) 2022, Florian Schuetz and Espressif Systems 
 *              (Shanghai) CO LTD (original code)
 * 
 * This code is an adaption of split_argv.c by Espressif Systems (Shanghai)
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
#include <ctype.h>
#include <string.h>

#define SS_FLAG_ESCAPE 0x8

typedef enum {
    /* parsing the space between arguments */
    SS_SPACE = 0x0,
    /* parsing an argument which isn't quoted */
    SS_ARG = 0x1,
    /* parsing a quoted argument */
    SS_QUOTED_ARG = 0x2,
    /* parsing an escape sequence within unquoted argument */
    SS_ARG_ESCAPED = SS_ARG | SS_FLAG_ESCAPE,
    /* parsing an escape sequence within a quoted argument */
    SS_QUOTED_ARG_ESCAPED = SS_QUOTED_ARG | SS_FLAG_ESCAPE,
} ch405_labs_split_state_t;

/* helper macro, called when done with an argument */
#define CH405_LABS_END_ARG() do { \
    char_out = 0; \
    argv[argc++] = next_arg_start; \
    state = SS_SPACE; \
} while(0)

size_t ch405_labs_esp_console_split_argv(char *line, char **argv, size_t argv_size)
{
    const int QUOTE = '"';
    const int ESCAPE = '\\';
    const int SPACE = ' ';
    ch405_labs_split_state_t state = SS_SPACE;
    size_t argc = 0;
    char *next_arg_start = line;
    char *out_ptr = line;
    for (char *in_ptr = line; argc < argv_size - 1; ++in_ptr) {
        int char_in = (unsigned char) *in_ptr;
        if (char_in == 0) {
            break;
        }
        int char_out = -1;

        switch (state) {
        case SS_SPACE:
            if (char_in == SPACE) {
                /* skip space */
            } else if (char_in == QUOTE) {
                next_arg_start = out_ptr;
                state = SS_QUOTED_ARG;
            } else if (char_in == ESCAPE) {
                next_arg_start = out_ptr;
                state = SS_ARG_ESCAPED;
            } else {
                next_arg_start = out_ptr;
                state = SS_ARG;
                char_out = char_in;
            }
            break;

        case SS_QUOTED_ARG:
            if (char_in == QUOTE) {
                CH405_LABS_END_ARG();
            } else if (char_in == ESCAPE) {
                state = SS_QUOTED_ARG_ESCAPED;
            } else {
                char_out = char_in;
            }
            break;

        case SS_ARG_ESCAPED:
        case SS_QUOTED_ARG_ESCAPED:
            if (char_in == ESCAPE || char_in == QUOTE || char_in == SPACE) {
                char_out = char_in;
            } else {
                /* unrecognized escape character, skip */
            }
            state = (ch405_labs_split_state_t) (state & (~SS_FLAG_ESCAPE));
            break;

        case SS_ARG:
            if (char_in == SPACE) {
                CH405_LABS_END_ARG();
            } else if (char_in == ESCAPE) {
                state = SS_ARG_ESCAPED;
            } else {
                char_out = char_in;
            }
            break;
        }
        /* need to output anything? */
        if (char_out >= 0) {
            *out_ptr = char_out;
            ++out_ptr;
        }
    }
    /* make sure the final argument is terminated */
    *out_ptr = 0;
    /* finalize the last argument */
    if (state != SS_SPACE && argc < argv_size - 1) {
        argv[argc++] = next_arg_start;
    }
    /* add a NULL at the end of argv */
    argv[argc] = NULL;

    return argc;
}
