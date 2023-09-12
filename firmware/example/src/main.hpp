/**
 * @file    main.h
 * @author  Florian Schütz (Grabmoix)
 * @brief   The main header file
 * @version 0.1
 * @date    12.09.2023
 * 
 * @copyright Copyright (c) 2023 Florian Schütz
 * 
 * This is an example firmware for the BalcCon Cyberdeck. It illustrates how
 * some of its functionality can be used. To start a clean project, use the 
 * framework. Make sure to consult the developer documentations.
 * 
 * TODO:
 *  - ensure all data buffers for transacttion over spi use pvPortMallocCaps(size, MALLOC_CAP_DMA)
 *  - maybe increase SPIRAM_MALLOC_RESERVE_INTERNAL to avoid allocation failure for DMA capable memory
 *  - move transactions that don't use dma and are not time critical to psram
 */

#include "sys/types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "sdkconfig.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_spiffs.h"
#include "nvs_flash.h"
#include "esp_vfs_dev.h"
#include <typeinfo>
#include <fstream>

#ifdef CONFIG_DISPLAY_SUPPORT
#include "st7735_bcd.hpp"
#include "gfx.hpp"
#include "ch405labs_gfx_menu.hpp"
#include "../fonts/Bm437_Acer_VGA_8x8.h"
#include "../fonts/Bm437_ACM_VGA_9x16.h"
#include "../fonts/Bm437_ATI_9x16.h"
#endif // CONFIG_DISPLAY_SUPPORT

#include "ch405labs_esp_controller.hpp"
#include "ch405labs_esp_led.hpp"
#include "ch405labs_esp_wifi.hpp"
#include "ch405labs_esp_console.hpp"
#include "ch405labs_esp_debug.h"
#include "ch405labs_esp_console.hpp"


// <----------------------------- Modules ------------------------------------->
//
// Add includes for the modules you use here
#include "mod_bcd_demo.hpp"
#include "mod_party.hpp"
#include "mod_logoslideshow.hpp"
#include "mod_saodemo.hpp"
#include "mod_settings.hpp"
#include "mod_snake.hpp"
#include "mod_cyberspace.hpp"

// <----------------------------- Commands ------------------------------------>
//
// Add commands for the modules you use here
#include "cmd_system.h"
#include "cmd_test.h"
#include "cmd_wifi.hpp"

// Namespaces
#ifdef CONFIG_DISPLAY_SUPPORT
using namespace espidf;
using namespace gfx;
#endif // CONFIG_DISPLAY_SUPPORT

//WLAN Configuration 
#define WIFI_SSID                           CONFIG_WIFI_SSID                    // Default access point
#define WIFI_PASS                           CONFIG_WIFI_PASSWORD                // Default access point password
#define WIFI_MAXIMUM_RETRY                  CONFIG_WIFI_MAXIMUM_RETRY


// Globals
static const char TAG_CHIP[] = "Chip";
static const char TAG_STATE[] = "State";
static const char TAG_FS[] = "Filesystem";
static const char TAG_COMMAND[] = "Command";
#ifdef CONFIG_DISPLAY_SUPPORT
static const char TAG_DISPLAY[] = "Display";
#endif // CONFIG_DISPLAY_SUPPORT


#ifdef CONFIG_DISPLAY_SUPPORT
// Display definitions
#define LCD_WIDTH       CONFIG_LCD_WIDTH  // 128
#define LCD_HEIGHT      CONFIG_LCD_HEIGHT // 160
#define LCD_ROTATION    3
// A note on the SPI bufffer. If buffer is too small (eg. 1/5 of the display
// size - TODO benchmarks needed), then its probably more efficient to disable
// the copy_from (which leads to using batching instead of blt)
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3
    #define SPI_BUFFER_SIZE      32768UL // As this is also the max transfer size on S3, TODO verify for S2,C3
#elif CONFIG_IDF_TARGET_ESP32
    #define SPI_BUFFER_SIZE      LCD_HEIGHT*LCD_WIDTH*2+8 // ESP32 max size big enough for full display
#endif //CONFIG_IDF_TARGET

#ifdef CONFIG_SPI2_HOST
    #define LCD_HOST    SPI3_HOST
#else 
    #define LCD_HOST    SPI4_HOST
#endif //CONFIG_SPI2_HOST

// TODO fix properly
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3
#define DMA_CHAN        SPI_DMA_CH_AUTO
#else // CONFIG_IDF_TARGET_ESP32
#define DMA_CHAN        CONFIG_LCD_DMA_CHANNEL
#endif //CONFIG_IDF_TARGET
#define PIN_NUM_MISO    (gpio_num_t)CONFIG_LCD_PIN_MISO 
#define PIN_NUM_MOSI    (gpio_num_t)CONFIG_LCD_PIN_MOSI
#define PIN_NUM_CLK     (gpio_num_t)CONFIG_LCD_PIN_CLK
#define PIN_NUM_CS      (gpio_num_t)CONFIG_LCD_PIN_CS

#define PIN_NUM_DC      (gpio_num_t)CONFIG_LCD_PIN_DC
#define PIN_NUM_RST     (gpio_num_t)CONFIG_LCD_PIN_RST
#define PIN_NUM_BCKL    (gpio_num_t)CONFIG_LCD_PIN_BCKL

// spi initialisation and global 
spi_master spi_host(nullptr,
                    LCD_HOST,
                    PIN_NUM_CLK,
                    PIN_NUM_MISO,
                    PIN_NUM_MOSI,
                    GPIO_NUM_NC,
                    GPIO_NUM_NC,
                    SPI_BUFFER_SIZE,
                    DMA_CHAN);

// we use the default, modest buffer - it makes things slower but uses less
// memory. it usually works fine at default but you can change it for performance 
// tuning. It's the final parameter: Note that it shouldn't be any bigger than 
// the DMA size
using lcd_type = st7735<LCD_WIDTH,
                        LCD_HEIGHT,
                        LCD_HOST,
                        PIN_NUM_CS,
                        PIN_NUM_DC,
                        PIN_NUM_RST,
                        PIN_NUM_BCKL,
                        LCD_ROTATION,
                        SPI_BUFFER_SIZE>;

lcd_type lcd; 

using lcd_color = color<typename lcd_type::pixel_type>;
#endif //CONFIG_DISPLAY_SUPPORT

struct hw_capabilities {
    bool fs_spiffs = true;
    bool led = true;
    bool display = 
#ifdef CONFIG_DISPLAY_SUPPORT
        true;
#else  
        false;
#endif // CONFIG_DISPLAY_SUPPORT
    bool wlan = true;
    bool console = true;
    bool controller = true;
} hwcap;



/////////////////////////////////////////////////////////////////
// Function prototypes
/////////////////////////////////////////////////////////////////
void uart_test();
#ifdef CONFIG_DISPLAY_SUPPORT
int config_display();
#endif // CONFIG_DISPLAY_SUPPORT
void lines_overlay();

extern "C" {
    void app_main(void);
}

/******************************************************************
 * Main
 * 
 * The main class configures all devices and the runs the main loop.
 ******************************************************************/
class Main final {
    private:
        controllerDriver controller;
        ledDriver& led = ledDriver::getInstance();
        espconsole::consoleController& console 
            = espconsole::consoleController::getInstance();
        TaskHandle_t cmdTaskHandle;

    public:
        void run(void);
        void setup(void);

        espwifi::wifiController::state_e wifiState 
            { espwifi::wifiController::state_e::NOT_INITIALIZED };
        espwifi::wifiController Wifi;
};