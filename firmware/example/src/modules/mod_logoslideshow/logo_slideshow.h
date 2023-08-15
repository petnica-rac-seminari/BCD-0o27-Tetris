#ifndef BCD_MODULE_LOGO_SLIDESHOW_HPP
#define BCD_MODULE_LOGO_SLIDESHOW_HPP

#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "st7735_bcd.hpp"
#include "gfx.hpp"
//#include "gfx_cpp14.hpp"
#include "fonts/Bm437_Acer_VGA_8x8.h"

#include "ch405labs_led.hpp"

#define LOGO_DISPLAY_TIME 2000
#define TAG_LOGO_SLIDESHOW "LOGO_SLIDESHOW"

using namespace espidf;
using namespace gfx;

template<typename Destination>
int logoSlideshow(void *param) {

// Bitmap for buffering
constexpr static const size16 bmp_size(160, 128); // width - height
using bmp_type = bitmap<rgb_pixel<16>>;
uint8_t *bmp_buf = (uint8_t *)malloc(bmp_type::sizeof_buffer(bmp_size)*sizeof(uint8_t));
bmp_type *bmp = NULL;
if(bmp_buf == NULL) {
    ESP_LOGW(TAG_LOGO_SLIDESHOW, "Not buffering display: Not enough free memory.");
} else {
    bmp = (bmp_type *)malloc(sizeof(bmp_type));
    if(bmp != NULL) {
        bmp = new (bmp) bmp_type(bmp_size, bmp_buf);
    } else {
        ESP_LOGW(TAG_LOGO_SLIDESHOW, "Not buffering display: Not enough free memory.");
        free(bmp_buf);
        bmp_buf = NULL;
    }
}

// LED things
ledDriver &led = ledDriver::getInstance();
LEDPatternGenerator generator;
ledStates pstates;
ledStates end_states;
ledPattern lp;
rgbLEDPixel led_color;

#ifdef CONFIG_DISPLAY_SUPPORT 

    Destination *lcd = (Destination *)param;

    // Clear the LCD   
    lcd->clear(lcd->bounds());
    if(bmp != NULL)
        bmp->clear(bmp->bounds());

    // Now display the logo

    // Display conference logo for booting.
    // First we need to open a file stream to the jpeg that contains
    // the logo. (GFX currently only supports jpeg)
    //
    // The jpeg has been previously put in the data directory of the
    // root folder of this project. We use spiffs as a filesystem, 
    // so the filesystem has been built using the "Build Filesystm
    // Image" command in the PlatformIO menu under project tasks ->
    // BCD-0o26 -> Platform. After successfully building the file-
    // system, the filesystem must be uplaoded using the "Upload
    // Filesystem Image" command in the same menu.
    char fnames[10][23] = {
        "/spiffs/logo_2k13.jpeg",
        "/spiffs/logo_2k14.jpeg",
        "/spiffs/logo_2k15.jpeg",
        "/spiffs/logo_2k16.jpeg",
        "/spiffs/logo_2k17.jpeg",
        "/spiffs/logo_2k18.jpeg",
        "/spiffs/logo_2k19.jpeg",
        "/spiffs/logo_2k20.jpeg",
        "/spiffs/logo_2k22.jpeg",
        "/spiffs/logo_2k23.jpeg"
    };
    for(int i = 0; i < 10; i++) {
        file_stream fs(fnames[i]);
        if(!fs.caps().read) {
            ESP_LOGE("SPIFFS", "Failed to open logo. Not showing....");
        } else {
            // Switch on leds
            generator.setInterruptable(false);
            generator.setRepetitions(1);

            // Caution: Led color order is green red blue --> Maybe fix lib
            switch(i) {
                case 0:
                    led_color = {
                        .red = 0x07,
                        .green = 0x0E,
                        .blue = 0x03
                    }; //yellowish-green
                    break;
                case 1:
                    led_color = {
                        .red = 0x12,
                        .green = 0x09,
                        .blue = 0x02
                    }; //orangeish
                    break;
                case 2:
                    led_color = {
                        .red = 0x03,
                        .green = 0x06,
                        .blue = 0x03
                    }; //pale green-blueish
                    break;
                case 3:
                    led_color = {
                        .red = 0x03,
                        .green = 0x03,
                        .blue = 0x09
                    }; //violett-blueish
                    break;
                case 4:
                    led_color = {
                        .red = 0x08,
                        .green = 0x00,
                        .blue = 0x00
                    }; //red
                    break;
                case 5:
                    led_color = {
                        .red = 0x05,
                        .green = 0x05,
                        .blue = 0x00
                    }; // pale yellow
                    break;
                case 6:
                    led_color = {
                        .red = 0x00,
                        .green = 0x00,
                        .blue = 0x05
                    }; //blue
                    break;
                case 7:
                    led_color = {
                        .red = 0x0D,
                        .green = 0x0D,
                        .blue = 0x0D
                    }; //whiteish
                    break;
                case 8:
                    led_color = {
                        .red = 0x09,
                        .green = 0x09,
                        .blue = 0x00
                    }; //yellow
                    break;
                case 9:
                    led_color = {
                        .red = 0x08,
                        .green = 0x00,
                        .blue = 0x0F
                    }; //violett
                    break;
                default:
                    led_color = {
                        .red = 0x0F,
                        .green = 0x0D,
                        .blue = 0x0B
                    };
                    break;
            }
            
            for(int j = 0; j < LED_IF_NUM_LED; j++) {
                pstates.led[j] = led_color;
            }
            generator.addState(pstates,pdMS_TO_TICKS(100));
            for(int j = 0; j < LED_IF_NUM_LED; j++) {
                end_states.led[j] = led_color;
            }
            generator.addEndState(end_states);
            generator.generate(&lp);
            

            if(bmp != NULL) {
                // Draw image on bitmap (buffering)
                draw::image(*bmp, (srect16)bmp->bounds(), &fs);
    
                // Schedule the led pattern    
                led_err_t led_error = led.patternSchedule(lp); // WARNING - never ever reuse a pattern as it might contain dynamically allocated memory that was freed by the thread.
                if(led_error != LED_OK) {
                    ESP_LOGE(TAG_LED_DRIVER, "Could not send pattern to LEDs.");
                }

                // Draw bmp on tft
                draw::bitmap(*lcd, (srect16)lcd->bounds(), *bmp, bmp->bounds());
            } else {
                // Schedule the led pattern
                led_err_t led_error = led.patternSchedule(lp); // WARNING - never ever reuse a pattern as it might contain dynamically allocated memory that was freed by the thread.
                if(led_error != LED_OK) {
                    ESP_LOGE(TAG_LED_DRIVER, "Could not send pattern to LEDs.");
                }
                
                // Draw image directly on tft
                draw::image(*lcd, (srect16)lcd->bounds(), &fs);
            }
            
            // Close the file
            fs.close();
        }

        vTaskDelay(pdMS_TO_TICKS(LOGO_DISPLAY_TIME));
    }
 #endif // CONFIG_DISPLAY_SUPPORT

    // Switch leds off
    generator.setInterruptable(false);
    generator.setRepetitions(1);

    for(int j = 0; j < LED_IF_NUM_LED; j++) {
        pstates.led[j] = {
            .red = 0x00,
            .green = 0x00,
            .blue = 0x00
        };
    }
    generator.addState(pstates,pdMS_TO_TICKS(100));
    for(int j = 0; j < LED_IF_NUM_LED; j++) {
        end_states.led[j] = {
            .red = 0x00,
            .green = 0x00,
            .blue = 0x00
        };
    }
    generator.addEndState(end_states);
    generator.generate(&lp);
    led_err_t led_error = led.patternSchedule(lp); // WARNING - never ever reuse a pattern as it might contain dynamically allocated memory that was freed by the thread.
    if(led_error != LED_OK) {
        ESP_LOGE(TAG_LED_DRIVER, "Could not send pattern to LEDs.");
    }

    // Delete the buffer bmp
    if(bmp != NULL) {
        bmp->~bmp_type();
        free(bmp);
    }
    if(bmp_buf != NULL) {
        free(bmp_buf);
    }

    return 0;  
}
#endif // BCD_MODULE_LOGO_SLIDESHOW_HPP