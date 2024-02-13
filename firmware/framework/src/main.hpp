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
#include "bcd_system.hpp"
#include "ch405labs_esp_debug.h"
#ifdef CONFIG_DISPLAY_SUPPORT
#include "ch405labs_gfx_menu.hpp"
#endif // CONFIG_DISPLAY_SUPPORT


#include "board.h"
#include "../fonts/Bm437_Acer_VGA_8x8.h"


// <----------------------------- Modules ------------------------------------->
//
// Add includes for the modules you use here
#include "mod_template.hpp"

// <----------------------------- Commands ------------------------------------>
//
// Add commands for the modules you use here
#include "cmd_template.hpp"


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
static const char TAG_STATE[] = "State";
static const char TAG_FS[] = "Filesystem";


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
#ifdef CONFIG_DISPLAY_SUPPORT
        lcd_type &lcd = bcd_sys.getDisplay();                                   /**< Display driver */
#endif //CONFIG_DISPLAY_SUPPORT
#ifdef CONFIG_CH405LABS_CONTROLLER_SUPPORT
        controllerDriver& controller = bcd_sys.getControllerDriver();           /**< Controller driver */
#endif //CONFIG_CH405LABS_CONTROLLER_SUPPORT
#ifdef CONFIG_LED_IF_SUPPORT
        ledDriver& led = bcd_sys.getLedDriver();                                /**< LED driver */
#endif //CONFIG_LED_IF_SUPPORT
        espconsole::consoleController& console = bcd_sys.getConsoleController();/**< Console controller */
        TaskHandle_t cmdTaskHandle;                                             /**< Task handle for cmd task*/

        espwifi::wifiController::state_e wifiState 
            { espwifi::wifiController::state_e::NOT_INITIALIZED };              /**< WiFi state */
        espwifi::wifiController &Wifi = bcd_sys.getWifiController();            /**< WiFi controller */

        tetrics_module::board board;

    public:
        void run(void);                                                         /**< Main loop */
        void setup(void);                                                       /**< Setup / initialisation code */

        void drawStartScreen();
        void drawGameScreen();
        void drawEndScreen();
};
