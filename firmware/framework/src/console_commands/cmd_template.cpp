/**
 * @file 	cmd_template.cpp
 * @author 	Florian Schuetz (fschuetz@ieee.org)
 * @brief 	Template for the implementation of console commands.
 * @version 	1.0
 * @date 	2023-08-07	
 * @copyright 	Copyright (c) 2023, Florian Schuetz, released under MIT license
 *
 * This file serves as a template to implement console commands. Check The 
 * header file cmd_template.hpp for details.
 */
#include "cmd_template.hpp"


static void register_cmd_1(void);
static void register_cmd_2(void);

/** register all commands in this library */
void register_cmd_template(void) {
    register_cmd_1();
    register_cmd_2();
}


/**
 * @brief cmd_1 does nothing 
 *
 * `cmd_1` command does nothing as this is a template.
 *
 * @param argc the number of given arguments
 * @param argv the vector of the arguments (0 terminated)
 * @return 0 if successful.
 */
int cmd_1(int argc, char **argv) {
    ESP_LOGI(TAG_CMD_TEMPLATE, "Do nothing.");
    
    return 0;
}

static void register_cmd_1(void) {
    const ch405_labs_esp_console_cmd_t cmd = {
        .command = "cmd_1",
        .help = "Do nothing, we are a template command.",
        .hint = NULL,
        .func = &cmd_1,
        .argtable = NULL,
    };
    ESP_ERROR_CHECK( ch405_labs_esp_console_cmd_register(&cmd) );
}


/**
 * @brief cmd_2 does nothing 
 *
 * `cmd_2` command does nothing as this is a template.
 *
 * @param argc the number of given arguments
 * @param argv the vector of the arguments (0 terminated)
 * @return 0 if successful.
 */
int cmd_2(int argc, char **argv) {
    ESP_LOGI(TAG_CMD_TEMPLATE, "Do nothing.");
    
    return 0;
}

static void register_cmd_2(void) {
    const ch405_labs_esp_console_cmd_t cmd = {
        .command = "cmd_2",
        .help = "Do nothing, we are a template command.",
        .hint = NULL,
        .func = &cmd_2,
        .argtable = NULL,
    };
    ESP_ERROR_CHECK( ch405_labs_esp_console_cmd_register(&cmd) );
}

