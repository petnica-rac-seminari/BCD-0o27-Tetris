/**
 * @file    main.hpp
 * @author  Florian Schütz (Grabmoix)
 * @brief   The main header file
 * @version 0.1
 * @date    2022-08-07
 * 
 * @copyright Copyright (c) 2022 Florian Schütz, released under MIT license
 * 
 * The main header file of the BalcCon Cyberdeck (BCD) defines the hardware
 * parameters and software modules and commands supported by the BCD. Unless you
 * are a framework developer, you only need to add the header files as includes
 * for your modules and commands. The two sections to do this are clearly marked
 * as <--- Modules --> and <--- Commands -->.
 * 
 * If you are a framework developer make sure to read the developer 
 * documentation.
 * 
 * TODO:
 *  - ensure all data buffers for transacttion over spi use 
 *      pvPortMallocCaps(size, MALLOC_CAP_DMA)
 *  - maybe increase SPIRAM_MALLOC_RESERVE_INTERNAL to avoid allocation failure 
 *      for DMA capable memory
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
#include "mod_template.hpp"

// <----------------------------- Commands ------------------------------------>
//
// Add commands for the modules you use here
#include "cmd_template.hpp"


// Namespaces
using namespace espidf;
#ifdef CONFIG_DISPLAY_SUPPORT
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
#define LCD_WIDTH       CONFIG_LCD_WIDTH  
#define LCD_HEIGHT      CONFIG_LCD_HEIGHT 
#define LCD_ROTATION    3
// A note on the SPI bufffer upper and lower bounds. 
//
// If buffer is too small (eg. 1/5 of the display size - TODO benchmarks 
// needed), then its probably more efficient to disable the copy_from (which 
// leads to using batching instead of blt). At the upper bound, the SPI buffer 
// shouldn't be any bigger than the DMA size
#if CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3 || CONFIG_IDF_TARGET_ESP32C3
    #define SPI_BUFFER_SIZE      32768UL                                        // Max transfer size on S3 (TODO verify S2,C3)
#elif CONFIG_IDF_TARGET_ESP32
    #define SPI_BUFFER_SIZE      LCD_HEIGHT*LCD_WIDTH*2+8                       // ESP32 max big enough for full display
#endif //CONFIG_IDF_TARGET

#ifdef CONFIG_SPI2_HOST
    #define LCD_HOST    SPI3_HOST
#else 
    #define LCD_HOST    SPI4_HOST
#endif //CONFIG_SPI2_HOST

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

// define type of the display
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

////////////////////////////////////////////////////////////////////////////////
// Programm entry point definition
////////////////////////////////////////////////////////////////////////////////
extern "C" {
    void app_main(void);
}

/**
 * @brief The main class to bootstrap everything
 * 
 * The main class contains setup routins, the main programm entry and the main
 * loop.
 */
class Main final {
    private:
        controllerDriver controller;                                            /**< Controller driver */
        ledDriver& led = ledDriver::getInstance();                              /**< LED driver */
        espconsole::consoleController& console =                                /**< Console controller */
            espconsole::consoleController::getInstance();
        TaskHandle_t cmdTaskHandle;                                             /**< Task handle for cmd task*/
        espwifi::wifiController::state_e wifiState {                            /**< WiFi state */
            espwifi::wifiController::state_e::NOT_INITIALIZED };                /**< WiFi controller */
        espwifi::wifiController Wifi;

    public:
        void run(void);                                                         /**< Main loop */
        void setup(void);                                                       /**< Setup / initialisation code */
};