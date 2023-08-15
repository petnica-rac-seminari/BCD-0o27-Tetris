/** 
 * @file        ch405_labs_led.cpp
 * @author      Florian Schütz (fschuetz@ieee.org)
 * @brief       Driver and pattern generations functions for driving addressable
 *              leds.
 * @version     0.9
 * @date        2022-08-07
 * @copyright   Copyright (c) 2022, Florian Schuetz, released under MIT license
 *
 * This file serves as a template to programm a module to be run on the BalcCon
 * cyberdeck. Consult the developer documentation and examples for more 
 * information.
 */

#pragma once
#ifndef BCD_MODULE_TEMPLATE_HPP
#define BCD_MODULE_TEMPALTE_HPP

#include <stdlib.h>
#include "esp_log.h"
#include "st7735_bcd.hpp"
#include "gfx.hpp"
#include "ch405labs_led.hpp"
#include "controller.hpp"

////////////////////////////////////////////////////////////////////////////////
// Menuconfig options
////////////////////////////////////////////////////////////////////////////////
#define TAG_MOD_TEMPLATE                   CONFIG_TAG_MOD_TEMPLATE

#if CONFIG_MOD_TEMPLATE_LOG_LEVEL == 0
#define MOD_TEMPLATE_LOG_LEVEL esp_log_level_t::ESP_LOG_NONE
#elif CONFIG_MOD_TEMPLATE_LOG_LEVEL == 1
#define MOD_TEMPLATE_LOG_LEVEL esp_log_level_t::ESP_LOG_ERROR
#elif CONFIG_MOD_TEMPLATE_LOG_LEVEL == 2
#define MOD_TEMPLATE_LOG_LEVEL esp_log_level_t::ESP_LOG_WARN
#elif CONFIG_MOD_TEMPLATE_LOG_LEVEL == 3
#define MOD_TEMPLATE_LOG_LEVEL esp_log_level_t::ESP_LOG_INFO
#elif CONFIG_MOD_TEMPLATE_LOG_LEVEL == 4
#define MOD_TEMPLATE_LOG_LEVEL esp_log_level_t::ESP_LOG_DEBUG
#elif CONFIG_MOD_TEMPLATE_LOG_LEVEL == 5
#define MOD_TEMPLATE_LOG_LEVEL esp_log_level_t::ESP_LOG_VERBOSE
#endif //CONFIG_MOD_TEMPLATE_LOG_LEVEL

////////////////////////////////////////////////////////////////////////////////
// Error handling
////////////////////////////////////////////////////////////////////////////////
typedef BaseType_t mod_template_err_t;

#define MOD_TEMPLATE_FAIL                                              -1          /**< Generic failure */
#define MOD_TEMPLATE_OK                                                0x000       /**< All good */


////////////////////////////////////////////////////////////////////////////////
// Macros
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Namespaces
////////////////////////////////////////////////////////////////////////////////
using namespace espidf;
using namespace gfx;

////////////////////////////////////////////////////////////////////////////////
// Types
////////////////////////////////////////////////////////////////////////////////
using pixel_type = rgb_pixel<16>;
using bmp_type = bitmap<pixel_type>;
using const_bmp_type = const_bitmap<pixel_type>;

namespace bcd_mod_template {
    ////////////////////////////////////////////////////////////////////////////
    // Main function
    ////////////////////////////////////////////////////////////////////////////
    template<typename Destination>
    int module_main(void *param) {

#ifdef CONFIG_DEBUG_STACK
        UBaseType_t uxHighWaterMark;

        /* Inspect our own high water mark on entering the task. */
        uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
        ESP_LOGD(CONFIG_TAG_STACK, "mod_snake::module_main(): High watermark for stack at start "
            "is: %d", uxHighWaterMark);
#endif

#ifdef CONFIG_DEBUG_HEAP
        multi_heap_info_t heap_info; 
        heap_caps_get_info(&heap_info, MALLOC_CAP_DEFAULT);
        ESP_LOGD(CONFIG_TAG_HEAP,   "demoMode(): Heap state at start: \n"
                                    "            Free blocks:           %d\n"
                                    "            Allocated blocks:      %d\n"
                                    "            Total blocks:          %d\n"
                                    "            Largest free block:    %d\n"
                                    "            Total free bystes:     %d\n"
                                    "            Total allocated bytes: %d\n"
                                    "            Minimum free bytes:    %d\n"
                                    , heap_info.free_blocks 
                                    , heap_info.allocated_blocks
                                    , heap_info.total_blocks
                                    , heap_info.largest_free_block
                                    , heap_info.total_free_bytes
                                    , heap_info.total_allocated_bytes
                                    , heap_info.minimum_free_bytes);
#endif // CONFIG_DEBUG_HEAP

	// PLACE CODE HERE
	//
	
 #ifdef CONFIG_DEBUG_STACK
         /* Inspect our own high water mark before exiting the task. */
         uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
         ESP_LOGD(CONFIG_TAG_STACK, "mod_snake::module_main(): High watermark for stack at start "
             "is: %d", uxHighWaterMark);
 #endif

	return 0;
    }
}
#endif // BCD_MODULE_SNAKE_HPP
