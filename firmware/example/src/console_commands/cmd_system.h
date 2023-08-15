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
#include <ctype.h>
#include "esp_log.h"
#include "ch405_labs_esp_console.h"
#include "esp_system.h"
#include "esp_sleep.h"
#include "spi_flash_mmap.h"
#include "driver/rtc_io.h"
#include "driver/uart.h"
#include "argtable3/argtable3.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#ifdef CONFIG_FREERTOS_USE_STATS_FORMATTING_FUNCTIONS
#define WITH_TASKS_INFO 1
#endif

#define ARRAY_SIZE_OFFSET   5   //Increase this if print_real_time_stats returns ESP_ERR_INVALID_SIZE

static const char TAG_COMMAND_SYSTENM[] = "cmd_system";

// Register system functions
void register_system(void);

int get_version(int argc, char **argv);
int restart(int argc, char **argv);
int free_mem(int argc, char **argv);
int heap_size(int argc, char **argv);

#if WITH_TASKS_INFO
int tasks_info(int argc, char **argv);
int stats_info(int argc, char **argv);
int interval_stats_info(int argc, char **argv);
#endif // WITH_TASKS_INFO

#ifdef __cplusplus
}
#endif
