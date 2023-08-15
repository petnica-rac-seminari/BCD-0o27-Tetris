/* Console example — various system commands

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/


#ifdef __cplusplus
extern "C" {
#endif

#pragma once

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "ch405_labs_esp_console.h"

static const char TAG_COMMAND_TEST[] = "cmd_test";

// Register system functions
void register_test(void);

#ifdef __cplusplus
}
#endif
