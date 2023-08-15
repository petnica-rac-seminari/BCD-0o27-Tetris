/* Console example — various system commands

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/




#pragma once

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "ch405_labs_esp_console.h"
#include "ch405labs_esp_wifi.hpp"

////////////////////////////////////////////////////////////////////////////////
// Error handling
////////////////////////////////////////////////////////////////////////////////

/** @typedef The error type for any wifi related error */
typedef BaseType_t wificmd_err_t;

#define WIFICMD_FAIL                      -1      /**< Generic failure */
#define WIFICMD_OK                        0x000   /**< Success */
#define WIFICMD_INVALID_ARG               0x010   /**< Invalid argument */

#define WIFICMD_INVALID_ARG_STRING        "Invalid argument"

////////////////////////////////////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////////////////////////////////////
static const char TAG_COMMAND_WIFI[] = "COMMAND WIFI";
static espwifi::wifiStaConfig staged_config;
static std::vector<espwifi::wifiStaConfig> loaded_configs;      

// Register system functions
void register_wifi(void);

int lsap(int argc, char **argv);
int wificfg(int argc, char **argv);
int apinfo(int argc, char **argv);
