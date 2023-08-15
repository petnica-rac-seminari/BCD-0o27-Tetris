#ifndef BCD_MODULE_DEMO_MODE_HPP
#define BCD_MODULE_DEMO_MODE_HPP

#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "st7735_bcd.hpp"
#include "gfx.hpp"
//#include "gfx_cpp14.hpp"
#include "fonts/Bm437_Acer_VGA_8x8.h"

#include "ch405labs_led.hpp"
#include "controller.hpp"

#include "modules/mod_logoslideshow/logo_slideshow.h"
#include "modules/mod_party/party.h"

#include "../../helpers/debug.hpp"

#define TAG_DEMO_MODE   "MOD_DEMO_MODE"

using namespace espidf;
using namespace gfx;
using namespace gfxmenu;


/**
 * @brief Delays approximately for the time given in ms while checking for controller
 *          events to abort every 100ms
 * 
 * @param delay The delay in ms
 * @param controller Pointer to the controller to check for abort button press
 * @return True if abort happened, false otherwise
 */
inline bool delayAndCheckAbort(uint16_t delay, controllerDriver *controller) {

#ifdef CONFIG_INPUT_SUPPORT

    // Delay and continously check if we should abort (only if controller 
    // configured, otherwise we just delay)
    if((*controller).getErrorState() == CONTROLLER_OK) {
        bool abort = false;
        uint16_t cycles = delay / 100;
        uint16_t remainder = delay % 100;
        for(int j = 0; j < cycles; j++) {
            // Check controller if we need to abort
            controller->capture();
            if(controller->getButtonState(BUTTON_X) 
                    || controller->getButtonState(BUTTON_Y)) { 
                abort = true;
                break;
            } 
            vTaskDelay(pdMS_TO_TICKS(100));
        }

        if(!abort && remainder > 0) {
            // Check controller if we need to abort

            controller->capture();
            if( controller->getButtonState(BUTTON_X) 
                    || controller->getButtonState(BUTTON_Y)) { 
                abort = true;
            } 
            vTaskDelay(pdMS_TO_TICKS(remainder));  
        }  

        // Check controller if we need to abort
        controller->capture();
        if((*controller).getButtonState(BUTTON_X) 
                || (*controller).getButtonState(BUTTON_Y)) { 
            abort = true;
        }
        return abort;

    } else {
#endif //CONFIG_INPUT_SUPPORT
        vTaskDelay(pdMS_TO_TICKS(delay));
        return false;
#ifdef CONFIG_INPUT_SUPPORT
    }
#endif //CONFIG_INPUT_SUPPORT
}

template<typename Destination>
int demoMode(void *param) {

#ifdef CONFIG_DEBUG_STACK
    UBaseType_t uxHighWaterMark;

    /* Inspect our own high water mark on entering the task. */
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    ESP_LOGD(CONFIG_TAG_STACK, "demoMode(): High watermark for stack at start "
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

#ifdef CONFIG_INPUT_SUPPORT
    controllerDriver controller;
    controller_err_t controller_err = controller.config();

    if(controller_err != CONTROLLER_OK && controller_err != CONTROLLER_ALREADY_CONFIGURED) {
        ESP_LOGE(TAG_CONTROLLER, "Controller not functioning. Cannot abort demo mode. (%d)", controller_err);
    }
#endif //CONFIG_INPUT_SUPPORT

#ifdef CONFIG_DISPLAY_SUPPORT 
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

        mc.cursor->addEntry(mc.createActionItem("Games", NULL, NULL));
        mc.cursor->addEntry(mc.createActionItem("Access Cyberspace", NULL, NULL));
        mc.cursor->addEntry(mc.createActionItem("Logo Slideshow", logoSlideshow<Destination>, lcd));
        mc.cursor->addEntry(mc.createActionItem("Party", discoFunction<Destination>, lcd));
        mc.cursor->addEntry(mc.createActionItem("Settings", NULL, NULL));

        mc.cursor->setToRoot();
        mc.cursor->first();
        mc.cursor->selectEntry();

        srect16 bounds = srect16(bmp->bounds());
        mc.cursor->drawMenu(*bmp, bounds, lcd_color::alice_blue, lcd_color::dark_goldenrod);
        draw::bitmap(*lcd, (srect16)lcd->bounds(), *bmp, bmp->bounds());

        vTaskDelay(pdMS_TO_TICKS(500));

        // Start the main loop
        int return_code = 0;
        bool abort = false;
        while(true) {
#ifdef CONFIG_DEBUG_HEAP
            heap_caps_get_info(&heap_info, MALLOC_CAP_DEFAULT);
            ESP_LOGD(CONFIG_TAG_HEAP,   "demoMode(): Heap state at beginning of loop: \n"
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

            // Step through the menu and execute item 3 and 4
            for(int i = 0; i < 4; i++) {
               
                // Move to next menu item
                mc.cursor->deselectEntry();
                mc.cursor->next();
                mc.cursor->selectEntry();
                bounds = srect16(bmp->bounds());
                mc.cursor->drawMenu(*bmp, bounds, lcd_color::alice_blue, lcd_color::dark_goldenrod);
                draw::bitmap(*lcd, (srect16)lcd->bounds(), *bmp, bmp->bounds());

                // Delay and continously check if we should abort
                abort = delayAndCheckAbort(500, &controller);
                if(abort) {
                    break;
                }

                if(i == 1 || i == 2) {
                    // Execute the menu item
                    // Note: We know the type is ActionItem in this demo mode, 
                    //       there is no need to dynamically check
                    const Entry<bmp_type, bmp_type::pixel_type> *e = mc.cursor->getEntry();
                    return_code = ((ActionItem<bmp_type, bmp_type::pixel_type> *)e)->execute();
                    if(return_code < 0) {
                        ESP_LOGE(TAG_DEMO_MODE, "Could not execute menu item. Code %d", return_code);
                    } else if (return_code > 0) {
                        ESP_LOGW(TAG_DEMO_MODE, "Menu execution returned with code %d.", return_code);
                    };
                }
            }

            if(abort) {
                break;
            }

            // Step back through menu up to first item
            for(int i = 0; i < 4; i++) {
                // Move to previous item
                mc.cursor->deselectEntry();
                mc.cursor->previous();
                mc.cursor->selectEntry();
                bounds = srect16(bmp->bounds());
                mc.cursor->drawMenu(*bmp, bounds, lcd_color::alice_blue, lcd_color::dark_goldenrod);
                draw::bitmap(*lcd, (srect16)lcd->bounds(), *bmp, bmp->bounds());
                
                // Delay and continously check if we should abort
                abort = delayAndCheckAbort(500, &controller);
                if(abort) {
                    break;
                }
            }

            if(abort) {
                break;
            }

            // Delay and continously check if we should abort, if not, resume
            // demo cycle
            abort = delayAndCheckAbort(500, &controller);
            if(abort) {
                break;
            }
        }

        // Free the bitmap and its buffer
#ifdef CONFIG_DEBUG_HEAP
        heap_caps_get_info(&heap_info, MALLOC_CAP_DEFAULT);
        ESP_LOGD(CONFIG_TAG_HEAP,   "demoMode(): Heap state before freeing display buffer: \n"
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

        bmp->~bmp_type();
        free(bmp); 
        free(bmp_buf); 

#ifdef CONFIG_DEBUG_HEAP
        heap_caps_get_info(&heap_info, MALLOC_CAP_DEFAULT);
        ESP_LOGD(CONFIG_TAG_HEAP,   "demoMode(): Heap state after freeing display buffer: \n"
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

    } else {
        // Do without buffering
        ESP_LOGW(TAG_GFXMENU, "Not buffering menu display: Not enough free memory.");
        MenuController<Destination, typename Destination::pixel_type> mc;

        mc.cursor->addEntry(mc.createActionItem("Games", NULL, NULL));
        mc.cursor->addEntry(mc.createActionItem("Access Cyberspace", NULL, NULL));
        mc.cursor->addEntry(mc.createActionItem("Logo Slideshow", logoSlideshow<Destination>, lcd));
        mc.cursor->addEntry(mc.createActionItem("Party", discoFunction<Destination>, lcd));
        mc.cursor->addEntry(mc.createActionItem("Settings", NULL, NULL));

        mc.cursor->setToRoot();
        mc.cursor->first();
        mc.cursor->selectEntry();

        srect16 bounds = srect16(lcd->bounds());
        mc.cursor->drawMenu(*lcd, bounds, lcd_color::alice_blue, lcd_color::dark_goldenrod);

        vTaskDelay(pdMS_TO_TICKS(500));

        // Start the main loop
        int return_code = 0;
        bool abort = false;
        while(true) {
            // Step through the menu and exectue item 3 and 4
            for(int i = 0; i < 4; i++) {
                // Move to next menu item
                mc.cursor->deselectEntry();
                mc.cursor->next();
                mc.cursor->selectEntry();
                bounds = srect16(lcd->bounds());
                mc.cursor->drawMenu(*lcd, bounds, lcd_color::alice_blue, lcd_color::dark_goldenrod);
                
                // Delay and continously check if we should abort
                abort = delayAndCheckAbort(500, &controller);
                if(abort) {
                    break;
                }

                if(i == 1 || i == 2) {
                    // Execute the menu item
                    // Note: We know the type is ActionItem in this demo mode, 
                    //       there is no need to dynamically check
                    const Entry<Destination, typename Destination::pixel_type> *e = mc.cursor->getEntry();
                    return_code = ((ActionItem<Destination, typename Destination::pixel_type> *)e)->execute();
                    if(return_code < 0) {
                        ESP_LOGE(TAG_DEMO_MODE, "Could not execute menu item. Code %d", return_code);
                    } else if (return_code > 0) {
                        ESP_LOGW(TAG_DEMO_MODE, "Menu execution returned with code %d.", return_code);
                    };
                }
            }

            if(abort) {
                break;
            }

            // Step back through menu up to first item
            for(int i = 0; i < 4; i++) {
                // Move to previous item
                mc.cursor->deselectEntry();
                mc.cursor->previous();
                mc.cursor->selectEntry();
                bounds = srect16(lcd->bounds());
                mc.cursor->drawMenu(*lcd, bounds, lcd_color::alice_blue, lcd_color::dark_goldenrod);
                
                // Delay and continously check if we should abort
                abort = delayAndCheckAbort(500, &controller);
                if(abort) {
                    break;
                }
            }
            
            if(abort) {
                break;
            }

            // Delay and continously check if we should abort, if not, resume
            // demo cycle
            abort = delayAndCheckAbort(500, &controller);
            if(abort) {
                break;
            }
        }
    }

#else // CONFIG_DISPLAY_SUPPORT

    logoSlideshow<Destination>(lcd);
    discoFunction<Destination> (lcd);

#endif // CONFIG_DISPLAY_SUPPORT

#ifdef CONFIG_DEBUG_STACK
    /* Inspect our own high water mark on entering the task. */
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    ESP_LOGD(CONFIG_TAG_STACK, "demoMode(): High watermark for stack at end."
        "is: %d", uxHighWaterMark);
#endif

    return 0;  
}

#endif //BCD_MODULE_DEMO_MODE_HPP