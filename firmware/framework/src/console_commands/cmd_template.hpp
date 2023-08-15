/** 
 * @file        cmd_template.hpp
 * @author      Florian Schuetz (fschuetz@ieee.org)
 * @brief      	Template for definiting console commands.. 
 * @version     1.0
 * @date        2023-08-07
 * @copyright   Copyright (c) 2023, Florian Schuetz, released under MTI license
 *
 * This file provides a template to develop console commands for the BalcCon 
 * Cyberdeck. Even though command is used in singualr throughout this template, 
 * a command template can register mutltiple commands. It is customary, that 
 * the commands programmed in one fail serve the same purpose. For example, 
 * commands in a wifi console command implementation should all serve the 
 * the purpose of manipulating WiFi. 
 * Adapt the template to your needs. At the minimum you should:
 *  - Rename the cmd_template.hpp and the cmd_template.cpp file into
 *  	cmd_<name of your command>.hpp / .cpp.
 *  - Change the error type from cmd_template_err_t to cmd_<name of your
 *  	command>_err_t,
 *  - Adatp TAG_CMD_TEMPLATE[] to TAG_CMD_<name of your command>[] and adapt the 
 *  	string to the name of your command.
 *  - Adapt the reigster function `void register_cmd_template(void)` to `void 
 *  	register_cmd_<name of your command>(void)`.
 *  - Replace function stubs `int cmd_1(int argc, char **argv);` and `int cmd_2 
 *  	(int argc, char **argv);` with the function definitions for your 
 *  	commands.
 *  - Implement the functions in the cmd_template.cpp file (respectively in 
 *  	teh file you renamed the cpp template to).
 *
 *  If your module should be used and extended by others, then it is advisable
 *  to allow setting the TAG and other options in menuconfig. Also you should
 *  incluude error level configuration through menuconfig and add some 
 *  functionality to allow for easier stack and heap diagnostics. 
 *  Also note, that if you use the display, there might be issues with 
 *  concurrency. Also, the display driver is templated and requires the code ot 
 *  be put in the header file.
 *  LEDs and the controller support concurrency, but you need to 
 *  be aware on the effects of using thos in a concurrent situation.
 *  Check the developer documentaion for details.
 */

#pragma once

#include <stdio.h>
#include "freertos/portmacro.h"
#include "esp_log.h"
#include "ch405_labs_esp_console.h"

////////////////////////////////////////////////////////////////////////////////
// Error handling
////////////////////////////////////////////////////////////////////////////////

/** @typedef The error type for any template related error */
typedef BaseType_t cmd_template_err_t;

#define CMD_TEMPLATE_FAIL                     1      /**< Generic failure */
#define CMD_TEMPLATE_OK                   0x000   /**< Success */


////////////////////////////////////////////////////////////////////////////////
// Globals
////////////////////////////////////////////////////////////////////////////////
static const char TAG_CMD_TEMPLATE[] = "COMMAND TEMPLATE";

// Register system functions
void register_cmd_template(void);

int cmd_1(int argc, char **argv);
int cmd_2(int argc, char **argv);
//etc...
