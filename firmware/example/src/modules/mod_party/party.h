#ifndef BCD_MODULE_PARTY_HPP
#define BCD_MODULE_PARTY_HPP

#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "st7735_bcd.hpp"
#include "gfx.hpp"
//#include "gfx_cpp14.hpp"
#include "fonts/Bm437_Acer_VGA_8x8.h"

#include "ch405labs_led.hpp"

#include "../../helpers/debug.hpp"

using namespace espidf;
using namespace gfx;

template<typename Destination>
int discoFunction(void *param) {

    Destination *lcd = (Destination *)param;

    ledDriver &led = ledDriver::getInstance();


#ifdef CONFIG_DISPLAY_SUPPORT    
    lcd->clear(lcd->bounds());
    const font& font = Bm437_Acer_VGA_8x8_FON;
    
    const char* text = "Welcome to the party\r\n  (disco  running)\r\n";
    srect16 text_rect = font.measure_text((ssize16)lcd->dimensions(),
                            text).bounds();
    draw::text(*lcd,
            text_rect.center((srect16)lcd->bounds()),
            text,
            font,
            color<typename Destination::pixel_type>::blue_violet);   
#endif // CONFIG_DISPLAY_SUPPORT

    // Generate first disco pattern
    LEDPatternGenerator generator;

    // First, we start with a very simple pattern of just blue, then 
    // green, then red. End state is to switch all leds off.
    generator.setInterruptable(false);
    generator.setRepetitions(2);

    ledStates pstates;
    for(int i = 0; i < LED_IF_NUM_LED; i++) {
        pstates.led[i] = {
            .red = 0x00,
            .green = 0x00,
            .blue = 0x00
        };
    }
    generator.addState(pstates,pdMS_TO_TICKS(500));

    for(int i = 0; i < LED_IF_NUM_LED; i++) {
        pstates.led[i] = {
            .red = 0x11,
            .green = 0x00,
            .blue = 0x00
        };
    }
    generator.addState(pstates, pdMS_TO_TICKS(250));

    for(int i = 0; i < LED_IF_NUM_LED; i++) {
        pstates.led[i] = {
            .red = 0x00,
            .green = 0x11,
            .blue = 0x00
        };
    }
    generator.addState(pstates, pdMS_TO_TICKS(500));

    ledPattern lp;
    generator.generate(&lp);
    led_err_t led_error = led.patternSchedule(lp); // WARNING - never ever reuse a pattern as it might contain dynamically allocated memory that was freed by the thread.
    
    if(led_error != LED_OK) {
        ESP_LOGE(TAG_LED_DRIVER, "Could not send pattern to LEDs.");
    }

    // Party pattern 2
    generator.reset(); // not strictly necessary as we generated the pattern which resets the generator.
    generator.setInterruptable(false);
    generator.setRepetitions(3);

    for(int i = 0; i < LED_IF_NUM_LED; i++) {
        pstates.led[i] = {
            .red = 0x11,
            .green = 0x00,
            .blue = 0x11,
        };
    }
    generator.addState(pstates, pdMS_TO_TICKS(250));

    for(int i = 0; i < LED_IF_NUM_LED; i++) {
        pstates.led[i] = {
            .red = 0x11,
            .green = 0x11,
            .blue = 0x00
        };
    }
    generator.addState(pstates, pdMS_TO_TICKS(500));

    for(int i = 0; i < LED_IF_NUM_LED; i++) {
        pstates.led[i] = {
            .red = 0x00,
            .green = 0x11,
            .blue = 0x11
        };
    }
    generator.addState(pstates, pdMS_TO_TICKS(250));

    generator.generate(&lp);
    led_error = led.patternSchedule(lp); // WARNING - never ever reuse a pattern as it might contain dynamically allocated memory that was freed by the thread.
    
    if(led_error != LED_OK) {
        ESP_LOGE(TAG_LED_DRIVER, "Could not send pattern to LEDs.");
    }

    // Party pattern 3
    generator.reset(); // not strictly necessary as we generated the pattern which resets the generator.
    generator.setInterruptable(true);
    generator.setRepetitions(4);

    for(int i = 0; i < LED_IF_NUM_LED; i++) {
        pstates.led[i] = {
            .red = 0x01,
            .green = 0x01,
            .blue = 0x11
        };
    }
    generator.addState(pstates, pdMS_TO_TICKS(125));

    for(int i = 0; i < LED_IF_NUM_LED; i++) {
        pstates.led[i] = {
            .red = 0x11,
            .green = 0x01,
            .blue = 0x01
        };
    }
    generator.addState(pstates, pdMS_TO_TICKS(125));

    for(int i = 0; i < LED_IF_NUM_LED; i++) {
        pstates.led[i] = {
            .red = 0x01,
            .green = 0x11,
            .blue = 0x01
        };
    }
    generator.addState(pstates, pdMS_TO_TICKS(125));

    ledStates end_states;
    for(int i = 0; i < LED_IF_NUM_LED; i++) {
        end_states.led[i] = {
            .red = 0x00,
            .green = 0x00,
            .blue = 0x00
        };
    }
    generator.addEndState(end_states);

    generator.generate(&lp);
    led_error = led.patternSchedule(lp); // WARNING - never ever reuse a pattern as it might contain dynamically allocated memory that was freed by the thread.
    
    if(led_error != LED_OK) {
        ESP_LOGE(TAG_LED_DRIVER, "Could not send pattern to LEDs.");
    }

    vTaskDelay(pdMS_TO_TICKS(5000));

#ifdef CONFIG_DISPLAY_SUPPORT    
    lcd->clear(lcd->bounds());

    const char* text_2 = "Party comes\r\n to an end\r\n";
    srect16 text_2_rect = font.measure_text((ssize16)lcd->dimensions(),
                            text_2).bounds();
    draw::text(*lcd,
            text_2_rect.center((srect16)lcd->bounds()),
            text_2,
            font,
            color<typename Destination::pixel_type>::chartreuse);   
#endif // CONFIG_DISPLAY_SUPPORT

    vTaskDelay(pdMS_TO_TICKS(2000));

    return 0;
}
#endif // BCD_MODULE_PARTY_HPP