#pragma once

#include "sdkconfig.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_spiffs.h"
#include "nvs_flash.h"
#include "esp_vfs_dev.h"

#define TAG_DISPLAY CONFIG_TAG_DISPLAY

#ifdef CONFIG_DISPLAY_SUPPORT
#include "gfx.hpp"
#include "st7735_bcd.hpp"
#include "../resources/bcd_default_font.hpp"
#endif // CONFIG_DISPLAY_SUPPORT

#include "ch405labs_esp_controller.hpp"
#include "ch405labs_esp_led.hpp"
#include "ch405labs_esp_wifi.hpp"
#include "ch405labs_esp_console.hpp"

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

// we use the default, modest buffer - it makes things slower but uses less
// memory. it usually works fine at default but you can change it for performance 
// tuning. It's the final parameter: Note that it shouldn't be any bigger than 
// the DMA size
using lcd_type = espidf::st7735<LCD_WIDTH,
                        LCD_HEIGHT,
                        LCD_HOST,
                        PIN_NUM_CS,
                        PIN_NUM_DC,
                        PIN_NUM_RST,
                        PIN_NUM_BCKL,
                        LCD_ROTATION,
                        SPI_BUFFER_SIZE>;

using lcd_color = gfx::color<typename lcd_type::pixel_type>;
#endif //CONFIG_DISPLAY_SUPPORT

/**
 * @brief A structure that represents the capabilities of the cyberdeck.
 * 
 * The structure is initialised by the chosen capabilities in menuconfig. It is
 * up to the user to update the structure representing actual capabilities. This
 * is usually done by initialising each hardware component and system compnent
 * and update the structure.
 */
class bcd_capabilities {
    public:
        bool fs_spiffs = true;

        bool led = 
#ifdef CONFIG_LED_IF_SUPPORT
        true;
#else //CONFIG_LED_IF_SUPPORT
        false;
#endif //CONFIG_LED_IF_SUPPORT

        bool display = 
#ifdef CONFIG_DISPLAY_SUPPORT
            true;
#else  
            false;
#endif // CONFIG_DISPLAY_SUPPORT

        bool wlan = true;
        bool console = true;
        bool controller = 
#ifdef CONFIG_CH405LABS_CONTROLLER_SUPPORT
            true;
#else
            false;
#endif //CONFIG_CH405LABS_CONTROLLER_SUPPORT
};

/**
 * @brief 
 * 
 * TODO
 *  - Introduce getLastError()
 */
class bcd_system {
    public:
        bcd_system() : spi_host(nullptr,
                        LCD_HOST,
                        PIN_NUM_CLK,
                        PIN_NUM_MISO,
                        PIN_NUM_MOSI,
                        GPIO_NUM_NC,
                        GPIO_NUM_NC,
                        SPI_BUFFER_SIZE,
                        DMA_CHAN) {};

        /**
         * @brief Checks if the cyberdeck supports a display
         * 
         * @return true if a display is supported
         * @return false otherwise
         */
        bool displaySupport();
        void setDisplaySupport(bool s);
#ifdef CONFIG_DISPLAY_SUPPORT
        lcd_type &getDisplay();
        espidf::spi_master &getSpiHost();
#endif //CONFIG_DISPLAY_SUPPORT

        bool ledSupport();
        void setLedSupport(bool s);
#ifdef CONFIG_LED_IF_SUPPORT
        ledDriver &getLedDriver();
#endif //CONFIG_LED_IF_SUPPORT

        bool spiffsSupport();
        void setSpiffsSupport(bool s);

        bool controllerSupport();
        void setControllerSupport(bool s);
#ifdef CONFIG_CH405LABS_CONTROLLER_SUPPORT
        controllerDriver &getControllerDriver();
#endif //CONFIG_CH405LABS_CONTROLLER_SUPPORT

        bool consoleSupport();
        void setConsoleSupport(bool s);
        espconsole::consoleController &getConsoleController();

        bool wifiSupport();
        void setWifiSupport(bool s);
        espwifi::wifiController &getWifiController();

    private:
        bcd_capabilities capabilities;
#ifdef CONFIG_DISPLAY_SUPPORT
        espidf::spi_master spi_host;                                            // object creation initialises spi 
        lcd_type lcd;
#endif //CONFIG_DISPLAY_SUPPORT
#ifdef CONFIG_LED_IF_SUPPORT
        ledDriver& led = ledDriver::getInstance();
#endif //CONFIG_LED_IF_SUPPORT
#ifdef CONFIG_CH405LABS_CONTROLLER_SUPPORT
        controllerDriver controller;
#endif //CONFIG_CH405LABS_CONTROLLER_SUPPORT
        espconsole::consoleController& console = 
            espconsole::consoleController::getInstance();
        espwifi::wifiController wifi;
};

extern bcd_system bcd_sys;


