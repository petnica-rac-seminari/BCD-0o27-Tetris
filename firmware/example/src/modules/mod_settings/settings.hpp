#ifndef BCD_MODULE_SETTINGS_HPP
#define BCD_MODULE_SETTINGS_HPP

#pragma once

#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include <stdlib.h>
#include "st7735_bcd.hpp"
#include "gfx.hpp"
//#include "gfx_cpp14.hpp"
#include "gfx_menu.hpp"
#include "../../fonts/Bm437_Acer_VGA_8x8.h"
#include "ch405labs_esp_wifi.hpp"

#include "ch405labs_led.hpp"


#include "../../helpers/debug.hpp"

#define TAG_SETTINGS   "SETTINGS"

using namespace espidf;
using namespace gfx;
using namespace gfxmenu;


/**
 * @brief Connect to wifi network
 * 
 * @param param 
 * @return int 
 */
template<typename Destination>
int connectWifi(void *param) {
    espwifi::wifiController Wifi;
    Destination *lcd = (Destination *)(((void **)param)[0]);
    espwifi::wifiStaConfig *cfg = (espwifi::wifiStaConfig *)(((void **)param)[1]);
    const font &font = Bm437_Acer_VGA_8x8_FON;

    // Messages and their sizes
    //                                          "12345678901234567890"
    const char *connection_msg                  =   "Connecting to ";
    ssize16 connection_msg_size = font.measure_text((ssize16)lcd->dimensions(), connection_msg);
    char ssid[32];
    cfg->getSsid(ssid, sizeof(ssid));
    rect16 lcd_bounds = lcd->bounds();
    lcd->clear(lcd_bounds);
    
    
    // Display connection attempt
    draw::text(*lcd,
                lcd_bounds,
                connection_msg,
                font,
                color<typename Destination::pixel_type>::red);   
    
    draw::text(*lcd,
                rect16(
                    lcd_bounds.x1,
                    lcd_bounds.y1 + connection_msg_size.height,
                    lcd_bounds.x2,
                    lcd_bounds.y2
                ),
                ssid,
                font,
                color<typename Destination::pixel_type>::red);

    // Test
    /*
    constexpr static const size16 sprite_size(16,16);
    // declare a monochrome bitmap.
    using mask_type = bitmap<gsc_pixel<1>>;
    uint8_t *mask_buf = NULL;
    mask_type *mask = NULL;  
    mask_buf = (uint8_t *)malloc(mask_type::sizeof_buffer(sprite_size)*sizeof(uint8_t));
    if(mask_buf != NULL) {
        mask = (mask_type *)malloc(sizeof(mask_type));
        if(mask != NULL) {
            mask = new (mask) mask_type(sprite_size, mask_buf);
            mask->clear(mask->bounds());
        } else {
            free(mask_buf);
            mask_buf = NULL;
        }
    } 

    // declare a color bitmap
    using bmp_type = bitmap<rgb_pixel<16>>;
    uint8_t *bmp_buf = NULL;
    bmp_type *bmp = NULL;  
    bmp_buf = (uint8_t *)malloc(bmp_type::sizeof_buffer(sprite_size)*sizeof(uint8_t));
    if(bmp_buf != NULL) {
        bmp = (bmp_type *)malloc(sizeof(bmp_type));
        if(bmp != NULL) {
            bmp = new (bmp) bmp_type(sprite_size, bmp_buf);
            bmp->clear(bmp->bounds());
        } else {
            free(bmp_buf);
            bmp_buf = NULL;
        }
    } 
  
    // draw the mask to the mask bitmap,
    draw::filled_rectangle()

    // and the sprite to the bmp bitmap



    // declare the sprite:
    using sprite_type = sprite<rgb_pixel<16>>;
    sprite_type sprite(sprite_size,bmp->begin(),mask->begin());

    // Draw the sprite
    //draw::sprite<>()
    */

    // Connect
    // If we are connected, disconnect
    espwifi::wifiController::state_e wifi_state = Wifi.GetState();
    if(wifi_state == espwifi::wifiController::state_e::CONNECTED ||
        wifi_state == espwifi::wifiController::state_e::CONNECTING ||
        wifi_state == espwifi::wifiController::state_e::WAITING_FOR_IP) {
        
        wifi_err_t status = Wifi.disconnect();
        if(status != ESP_OK) {
            ESP_LOGE(TAG_WIFI, "Could not disconnect from AP.");
            return -1; // TODO proper error code
        }

        // Wait at most 2.5 seconds for disconnect
        for(int i = 0; i < 5; i++) {
            wifi_state = Wifi.GetState();
            if(wifi_state != espwifi::wifiController::state_e::DISCONNECTED || 
                wifi_state != espwifi::wifiController::state_e::READY_TO_CONNECT) {       
                vTaskDelay(pdMS_TO_TICKS(500));
            } else {
                break;
            }
        }

    }

    // Apply staged config to wifiController
    Wifi.setStaConfig(*cfg);

    // Connect
    wifi_err_t status = Wifi.connect();
    if(status != WIFI_OK) {
        ESP_LOGE(TAG_WIFI, "Failed to connect.");
        return -1;
    }

    return 0;

}


template<typename Destination>
int selectWifi(void *param) {

#ifdef CONFIG_DEBUG_STACK
    UBaseType_t uxHighWaterMark;

    /* Inspect our own high water mark on entering the task. */
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    ESP_LOGD(CONFIG_TAG_STACK, "selectWifi(): High watermark for stack at start "
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

    
#if !defined(CONFIG_INPUT_SUPPORT) || !defined(CONFIG_DISPLAY_SUPPORT)
    // If input or display not supported abort.
    ESP_LOGE(TAG_SETTINGS, "Input or display not supported. Aborting...");
    return -1;
#endif
    
    // Load wifi configuration
    espwifi::wifiController Wifi;
    static std::vector<espwifi::wifiStaConfig> loaded_configs; 

    wifi_err_t err = Wifi.loadConfigs(loaded_configs);
    if(err != WIFI_OK) {
        // TODO show error on screen
        std::cout << "Failed loading configurations (" << err << ")" << std::endl;
        return -1;
    } else {
        std::cout << "Successfully loaded configurations." << std::endl;
    }
    
    controllerDriver controller;
    controller_err_t controller_err = controller.config();

    if(controller_err != CONTROLLER_OK && controller_err != CONTROLLER_ALREADY_CONFIGURED) {
        ESP_LOGE(TAG_CONTROLLER, "Controller not functioning. (%d)", controller_err);
        return -1;
    }


    Destination *lcd = (Destination *)param;
    using lcd_color = color<typename Destination::pixel_type>;

    // Clear the LCD   
    lcd->clear(lcd->bounds());

    gfx::rect16 dst_bounds = rect16(lcd->bounds());
    static const size16 bmp_size(dst_bounds.width(), dst_bounds.height()); 

    // Try to allocate memory for buffering display output
    bool buffering = false;
    using bmp_type = bitmap<rgb_pixel<16>>;
    uint8_t *bmp_buf = NULL;
    bmp_type *bmp = NULL;
    
    bmp_buf = (uint8_t *)malloc(bmp_type::sizeof_buffer(bmp_size)*sizeof(uint8_t));
    if(bmp_buf != NULL) {
        bmp = (bmp_type *)malloc(sizeof(bmp_type));
        if(bmp != NULL) {
            bmp = new (bmp) bmp_type(bmp_size, bmp_buf);
            bmp->clear(bmp->bounds());
            buffering = true;
        } else {
            free(bmp_buf);
            bmp_buf = NULL;
        }
    } 

    if(buffering) {

        MenuController<bmp_type, bmp_type::pixel_type> mc;

        // Display stored wifi networks
        std::vector<espwifi::wifiStaConfig>::iterator cfg_iter;
        for(cfg_iter = loaded_configs.begin(); cfg_iter < loaded_configs.end(); cfg_iter++) {
            // TODO add menu item for each network config
            char ssid[32];
            cfg_iter->getSsid(ssid, sizeof(ssid));
            void *args[2] = {(void*)lcd, (void*)&*cfg_iter};
            mc.cursor->addEntry(mc.createActionItem(ssid, connectWifi<Destination>, args, sizeof(args)));
        }

        mc.cursor->setToRoot();
        mc.cursor->first();
        mc.cursor->selectEntry();

        srect16 bounds = srect16(bmp->bounds());
        mc.cursor->drawMenu(*bmp, bounds, lcd_color::alice_blue, lcd_color::dark_goldenrod);
        draw::bitmap(*lcd, (srect16)lcd->bounds(), *bmp, bmp->bounds());

        while(true) {

            controller.capture();
            
            if(controller.getButtonState(BUTTON_DOWN)) {
                mc.cursor->deselectEntry();
                mc.cursor->next();
                mc.cursor->selectEntry();
                bounds = srect16(bmp->bounds());
                mc.cursor->drawMenu(*bmp, bounds, lcd_color::alice_blue, lcd_color::dark_goldenrod);
            } else if(controller.getButtonState(BUTTON_UP)) {
                mc.cursor->deselectEntry();
                mc.cursor->previous();
                mc.cursor->selectEntry();
                bounds = srect16(bmp->bounds());
                mc.cursor->drawMenu(*bmp, bounds, lcd_color::alice_blue, lcd_color::dark_goldenrod);
            } 

            if(controller.getButtonState(BUTTON_A) || controller.getButtonState(BUTTON_B)) {
                const Entry<bmp_type, bmp_type::pixel_type> *e = mc.cursor->getEntry();
                if(e != NULL && typeid(*e) == typeid(ActionItem<bmp_type, bmp_type::pixel_type>)) {
                    int return_code = ((ActionItem<bmp_type, bmp_type::pixel_type> *)e)->execute();
                    if(return_code < 0) {
                    ESP_LOGE(TAG_SETTINGS, "Could not execute menut item. Code %d", return_code);
                    } else if (return_code > 0) {
                        ESP_LOGW(TAG_SETTINGS,"Menu execution returned with code %d.", return_code);
                    };
                    // Need to draw menu again, as execution could have used screen
                    bounds = srect16(bmp->bounds());
                    mc.cursor->drawMenu(*bmp, bounds, lcd_color::alice_blue, lcd_color::dark_goldenrod);
                } else if (e != NULL && typeid(*e) == typeid(Submenu<bmp_type, bmp_type::pixel_type>)) {
                    mc.cursor->enter();
                    if(mc.cursor->first() == GFXMENU_OK) {
                        mc.cursor->selectEntry();
                    }
                    bounds = srect16(bmp->bounds());
                    mc.cursor->drawMenu(*bmp, bounds, lcd_color::alice_blue, lcd_color::dark_goldenrod);
                }
            }

            if(controller.getButtonState(BUTTON_X) || controller.getButtonState(BUTTON_Y)) { 
                // We do not have submenus here, so just break the loop. TODO check in future
                break;
            }
            draw::bitmap(*lcd, (srect16)lcd->bounds(), *bmp, bmp->bounds());
            vTaskDelay(pdMS_TO_TICKS(150));
        }

        delete bmp;
        free(bmp_buf);

    } else {
        ESP_LOGE(TAG_SETTINGS, "Unbuffered display not implemented.");
    }
    
    return 0;
}
#endif // BCD_MODULE_SETTINGS_HPP