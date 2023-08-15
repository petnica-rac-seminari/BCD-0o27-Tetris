/**
 * @file ch405labs_linennoise.c
 * @author Florian Schütz (fschuetz@ieee.org)
 * @brief Functions to manage a serial console on esp32.
 * @version 0.1
 * @date 2023-05-01
 * @copyright Modifications copyright (c) 2023 Florian Schütz, released under MIT license
 * @copyright Original code copyright (c) 2010-2023, Salvatore Snfilippo, under the BSD 2-Clause "Simplified" license
 * @copyright Original code copyright (c) 2010-2013, Pieter Noordhuis, under the BSD 2-Clause "Simplified" license 
 * 
 * This is an adaption of the linenoise library originally found at 
 * https://github.com/antirez/linenoise. It also uses some code snippets for 
 * determining ansi capability of the terminal and positioning from the adaption
 * of Espressif released under https://github.com/espressif/esp-idf/tree/master/components/console/linenoise
 * 
 * All function names have been adapted to avoid clashing with the version
 * included in espidf.
 *
 * ------------------------------------------------------------------------
 *
 * Copyright (c) 2010-2023, Salvatore Sanfilippo <antirez at gmail dot com>
 * Copyright (c) 2010-2013, Pieter Noordhuis <pcnoordhuis at gmail dot com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __CH405_LABS_LINENOISE_H
#define __CH405_LABS_LINENOISE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h> /* For bool. */
#include <stddef.h> /* For size_t. */
#include "esp_log.h"        // TODO - maybe we should detect the platform and not assume esp

#define TAG_LINENOISE     "LINENOISE"

extern const char *ch405_labs_linenoiseEditMore;

/* The ch405_labs_linenoiseState structure represents the state during line editing.
 * We pass this state to functions implementing specific editing
 * functionalities. */
struct ch405_labs_linenoiseState {
    int in_completion;  /* The user pressed TAB and we are now in completion
                         * mode, so input is handled by completeLine(). */
    size_t completion_idx; /* Index of next completion to propose. */
    int ifd;            /* Terminal stdin file descriptor. */
    int ofd;            /* Terminal stdout file descriptor. */
    char *buf;          /* Edited line buffer. */
    size_t buflen;      /* Edited line buffer size. */
    const char *prompt; /* Prompt to display. */
    size_t plen;        /* Prompt length. */
    size_t pos;         /* Current cursor position. */
    size_t oldpos;      /* Previous refresh cursor position. */
    size_t len;         /* Current edited line length. */
    size_t cols;        /* Number of columns in terminal. */
    size_t oldrows;     /* Rows used by last refrehsed line (multiline mode) */
    int history_index;  /* The history index we are currently editing. */
};

typedef struct ch405_labs_linenoiseCompletions {
  size_t len;
  char **cvec;
} ch405_labs_linenoiseCompletions;

/* Non blocking API. */
int ch405_labs_linenoiseEditStart(struct ch405_labs_linenoiseState *l, int stdin_fd, int stdout_fd, char *buf, size_t buflen, const char *prompt);
char *ch405_labs_linenoiseEditFeed(struct ch405_labs_linenoiseState *l);
void ch405_labs_linenoiseEditStop(struct ch405_labs_linenoiseState *l);
void ch405_labs_linenoiseHide(struct ch405_labs_linenoiseState *l);
void ch405_labs_linenoiseShow(struct ch405_labs_linenoiseState *l);

/* Blocking API. */
char *ch405_labs_linenoise(const char *prompt);
void ch405_labs_linenoiseFree(void *ptr);

/* Completion API. */
typedef void(ch405_labs_linenoiseCompletionCallback)(const char *, ch405_labs_linenoiseCompletions *);
typedef char*(ch405_labs_linenoiseHintsCallback)(const char *, int *color, int *bold);
typedef void(ch405_labs_linenoiseFreeHintsCallback)(void *);
void ch405_labs_linenoiseSetCompletionCallback(ch405_labs_linenoiseCompletionCallback *);
void ch405_labs_linenoiseSetHintsCallback(ch405_labs_linenoiseHintsCallback *);
void ch405_labs_linenoiseSetFreeHintsCallback(ch405_labs_linenoiseFreeHintsCallback *);
void ch405_labs_linenoiseAddCompletion(ch405_labs_linenoiseCompletions *, const char *);

/* History API. */
int ch405_labs_linenoiseHistoryAdd(const char *line);
int ch405_labs_linenoiseHistorySetMaxLen(int len);
int ch405_labs_linenoiseHistorySave(const char *filename);
int ch405_labs_linenoiseHistoryLoad(const char *filename);

/* Other utilities. */
void ch405_labs_linenoiseClearScreen(void);
void ch405_labs_linenoiseSetMultiLine(int ml);
void ch405_labs_linenoisePrintKeyCodes(void);
void ch405_labs_linenoiseMaskModeEnable(void);
void ch405_labs_linenoiseMaskModeDisable(void);
int ch405_labs_linenoiseSetMaxLineLen(size_t len);
int ch405_labs_linenoiseProbe(void);
void ch405_labs_linenoiseSetDumbMode(int set);
bool ch405_labs_linenoiseIsDumbMode(void);

#ifdef __cplusplus
}
#endif

#endif /* __CH405_LABS_LINENOISE_H */