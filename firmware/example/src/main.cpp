#include "main.h"
#include "led_patterns/led_patterns.h"


#ifdef CONFIG_DISPLAY_SUPPORT
/****************************************************************
 * Config Display
 * 
 * Configure the display.
 * 
 * TODO:
 *  - Proper error checking
 ****************************************************************/
int config_display() {
    // Variable for error tracking
    int error = ESP_OK;
    
    return error;
}

/****************************************************************
 * Lines overlay
 * 
 * Display some lines on the lcd.
 * 
 * TODO:
 *  - Proper error checking
 ****************************************************************/
void lines_overlay() {
    const font& f = Bm437_Acer_VGA_8x8_FON;
    const char* text = "BCD-0o27";
    srect16 text_rect = f.measure_text((ssize16)lcd.dimensions(),
                            text).bounds();

    draw::text(lcd,
            text_rect.center((srect16)lcd.bounds()),
            text,
            f,
            lcd_color::alice_blue);

    for(int i = 1;i<100;i+=2) {
        // calculate our extents
        srect16 r(i*(lcd_type::width/100.0),
                i*(lcd_type::height/100.0),
                lcd_type::width-i*(lcd_type::width/100.0)-1,
                lcd_type::height-i*(lcd_type::height/100.0)-1);
        // draw the four lines
        draw::line(lcd,srect16(0,r.y1,r.x1,lcd_type::height-1),lcd_color::light_blue);
        draw::line(lcd,srect16(r.x2,0,lcd_type::width-1,r.y2),lcd_color::hot_pink);
        draw::line(lcd,srect16(0,r.y2,r.x1,0),lcd_color::pale_green);
        draw::line(lcd,srect16(lcd_type::width-1,r.y1,r.x2,lcd_type::height-1),lcd_color::yellow);
        // the ESP32 wdt will get tickled
        // unless we do this:
        vTaskDelay(1);
    }
}
#endif //CONFIG_DISPLAY_SUPPORT

/****************************************************************
 * Main App
 ****************************************************************/
Main App;

void Main::run(void) {
#ifdef CONFIG_DEBUG_STACK
    UBaseType_t uxHighWaterMark;

    /* Inspect our own high water mark on entering the task. */
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    ESP_LOGD(TAG_STACK, "Main:run(): High watermark for stack at start is: %d", uxHighWaterMark);
#endif

    
    // We start a state machine and start in its root state
    //
    // At this point we initialised all the peripherals and their drivers. They
    // are all ready to be used at this point.
    //
    // NOTE: To use the framework, add menu items and modules. Of course, you
    //       can also customize the whole main loop and do not need to use the
    //       provided one.


    // --> You might want to put you logic below here. Unless you know what you
    //     are doing, you do not need to change any other code of the framwork
    //     outside this top and the bottom marker. Except of course the modules
    //     you add under ./modules
    

    // 0 Display a welcome screen
    //
    // For this we start the led pattern scheduler and load a pattern. Then we
    // display the conference logo and display an overlay animation.

    led_err_t led_error = led.patternStart();
    ledPattern lp;
    lp.repetitions = 1;
    lp.interruptable = true;
    lp.pattern_function = bootupPattern;
    lp.use_end_state = false;                             

    led_error = led.patternSchedule(lp);
    if(led_error != LED_OK) {
        ESP_LOGE(TAG_LED_DRIVER, "Could not send pattern to LEDs.");
    }

#ifdef CONFIG_DISPLAY_SUPPORT
    // Display conference logo for booting.
    // First we need to open a file stream to the jpeg that contains
    // the logo. (GFX currently only supports jpeg)
    file_stream fs("/spiffs/logo_2k23.jpeg");
    if(!fs.caps().read) {
        ESP_LOGE(TAG_FS, "Failed to open logo. Not showing....");
    } else {
        // To display the image we need to call jpeg_image::load with a
        // callback function that draws a portion of the image. We 
        // specify this callback directly as an argument.
        jpeg_image::load(&fs,[](size16 dimensions,
                                typename jpeg_image::region_type& region,
                                point16 location,
                                void* state) {
            // use draw:: to render this portion to the display
            return draw::bitmap(lcd, // lcd is available globally
                                srect16((spoint16)location,
                                        (ssize16)region.dimensions()),
                                        region,region.bounds());
        // we don't need state, so just use nullptr
        },nullptr);

        // Display the logo for half a second before moving on
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    // Draw the overlay animation
    lines_overlay(); 


    // Clear screen
    lcd.clear(lcd.bounds());

    // Build menu
    gfx::rect16 dst_bounds = rect16(lcd.bounds());
    static const size16 bmp_size(dst_bounds.width(), dst_bounds.height()); 

    using bmp_type = bitmap<rgb_pixel<16>>;
    uint8_t *bmp_buf = NULL;
    bmp_type *bmp = NULL;
    
    bmp_buf = (uint8_t *)malloc(bmp_type::sizeof_buffer(bmp_size)*sizeof(uint8_t));
    if(bmp_buf == 0) {
        ESP_LOGW(TAG_GFXMENU, "Not buffering menu display: Not enough free memory.");
        // No buffering
        MenuController<lcd_type, lcd_type::pixel_type> mc;

    } else {
        bmp = new bmp_type(bmp_size, bmp_buf);
        bmp->clear(bmp->bounds());

        MenuController<bmp_type, bmp_type::pixel_type> mc;

        // Games Submenu
        mc.cursor->addEntry(mc.createSubmenu("Games"));
        mc.cursor->enter();
        mc.cursor->addEntry(mc.createActionItem("Snake", bcd_mod_snake::module_main<lcd_type>, &lcd));
        mc.cursor->leave();

        // Rest of main menu
        void *args[2] = {(void*)&lcd, (void*)&cmdTaskHandle};
        mc.cursor->addEntry(mc.createActionItem("Access Cyberspace", bcd_cyberspace::sshConnectFunction<lcd_type>, args));
        mc.cursor->addEntry(mc.createActionItem("Logo Slideshow", logoSlideshow<lcd_type>, &lcd));
        mc.cursor->addEntry(mc.createActionItem("Party", discoFunction<lcd_type>, &lcd));
        mc.cursor->addEntry(mc.createActionItem("SAO Test", saoBlink<lcd_type>, &lcd));
        mc.cursor->addEntry(mc.createActionItem("Demo Mode", demoMode<lcd_type>, &lcd));

        // Settings Submenu
        mc.cursor->addEntry(mc.createSubmenu("Settings"));
        mc.cursor->enter();
        mc.cursor->addEntry(mc.createActionItem("Connect Wifi", selectWifi<lcd_type>, &lcd));
        mc.cursor->leave();

        // Reset cursor and select first entry
        mc.cursor->setToRoot();
        mc.cursor->first();
        mc.cursor->selectEntry();

        srect16 bounds = srect16(bmp->bounds());
        mc.cursor->drawMenu(*bmp, bounds, lcd_color::alice_blue, lcd_color::dark_goldenrod);

        // 2 Start the main loop. This loop must never exit!
        ESP_LOGV(TAG_STATE, "Starting main loop.");
        bounds = srect16(bmp->bounds());
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
                    ESP_LOGE(TAG_STATE, "Could not execute menut item. Code %d", return_code);
                    } else if (return_code > 0) {
                        ESP_LOGW(TAG_STATE,"Menu execution returned with code %d.", return_code);
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
                // Only redraw if we are not in the main menu
                mc.cursor->deselectEntry();
                if(mc.cursor->leave() == GFXMENU_OK) {
                    bounds = srect16(lcd.bounds());
                    mc.cursor->drawMenu(*bmp, bounds, lcd_color::alice_blue, lcd_color::dark_goldenrod);
                }
            }
            draw::bitmap(lcd, (srect16)lcd.bounds(), *bmp, bmp->bounds());
            vTaskDelay(pdMS_TO_TICKS(150));
        }
        delete bmp;
        free(bmp_buf);
        
    }
    // TODO remove guard
    while(true) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        #ifdef CONFIG_DEBUG_STACK
        /* Inspect our own high water mark on entering the task. */
        uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
        ESP_LOGD(TAG_STACK, "Main:run(): High watermark for stack in idle guard is: %d", uxHighWaterMark);
        #endif  
    };
#endif // CONFIG_DISPLAY_SUPPORT
   

// <-- You might want to put you logic above here. Unless you know what 
//     you are doing, you do not need to change any other code of the 
//     framwork outside this bottom and the top marker.

#ifdef CONFIG_DEBUG_STACK
/* Inspect our own high water mark on entering the task. */
uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
ESP_LOGD(TAG_STACK, "Main:run(): High watermark for stack after after state execution is: %d", uxHighWaterMark);
#endif  
}

/**
 * @brief The setup function for the badge
 * 
 * This function initilises all the hardware on the badge and does other
 * preparatory tasks, such as connecting to WiFi. 
 * 
 * We start by loading the minimal drivers to display the conference logo and
 * some loading sequence. This requires the led drivers, display drivers and the
 * led drivers to be set up. After displaying the loading screen, we initialise
 * the rest of the hardware and complete software setup.
 * 
 * Sequence:
 *      0. Leds
 *      1. Display setup
 *      2. Filesystem setup:
 *          - SPIFFS 
 *          - NVS
 *      3. Controller (push buttons)
 *      4. Console
 *      5. WiFi
 * 
 * Note: When setting the leds we do not check for the scheduler state. We get
 *          away with this as we just initialised the leds and we know that 
 *          the scheduler is stopped. Also we know that there is no other thread
 *          that can start the scheduler at this point in time.
 * 
 * TODO:
 *  - Build a structure to reflect which devices / resources may be used and 
 *      which failed.
 */
void Main::setup(void) {
#ifdef CONFIG_DEBUG_STACK
    UBaseType_t uxHighWaterMark;

    /* Inspect our own high water mark on entering the task. */
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    ESP_LOGD(TAG_STACK, "Main:setup(): High watermark for stack at start is: %d", uxHighWaterMark);
#endif

    // LED colors we us throughout the routing
    rgbLEDPixel red = {
        .red = 0x0F,
        .green = 0x00,
        .blue = 0x00
    };

    rgbLEDPixel green = {
        .red = 0x00,
        .green = 0x0F,
        .blue = 0x00
    };

    rgbLEDPixel clear = {
        .red = 0x00,
        .green = 0x00,
        .blue = 0x00
    };

    // Led state to indicate a fatal error.
    ledStates led_fatal_error;
    for(int i = 0; i < LED_IF_NUM_LED; i++) {
        led_fatal_error.led[i] = red;      
    }


    // 0. Setup LEDs
    //
    // First we try to set up the leds. (We will use the leds to display bootup
    // status.)
    led_err_t led_error = led.getStatus();

    if(led_error != LED_OK) {
        ESP_LOGV(TAG_LED_DRIVER, "Driver in error state (E:0x%x): Resetting...", led_error);
        led_error = led.reset();
    } 

    if(led_error != LED_OK) {
        hwcap.led = false;
        ESP_LOGE(TAG_LED_DRIVER, "Could not configure leds. Running without"
            " led support. (E:0x%x)", led_error);
    } else {
        // Clear out all the leds
        ledStates clearLeds;
        for(int i = 0; i < LED_IF_NUM_LED; i++) {
            clearLeds.led[i] = clear;
        }
        led_error = led.setLeds(clearLeds);
        vTaskDelay(pdMS_TO_TICKS(500));  //Give the rmt driver some time TODO - 200ms?

        // Now switch first led to green to show successful led setup
        led_error = led.setLed(0, green);
        vTaskDelay(pdMS_TO_TICKS(500));  //Let us enjoy the led 500ms

        if(led_error != LED_OK) {
            ESP_LOGW(TAG_LED_DRIVER, "setLED failed (0x%x)", led_error);
            led_error = LED_OK; // needed to not display initialisation error
        }
    }

#ifdef CONFIG_DISPLAY_SUPPORT
    // 1. Display setup 
    // 
    // If the badge supports a display we initialise it here and clear the 
    // screen.
    if(!spi_host.initialized()) {
        ESP_LOGE(TAG_DISPLAY, "SPI host initialization error. Halting...\r\n");
        if(hwcap.led) {
            led.setLeds(led_fatal_error);
        }
        vTaskDelay(portMAX_DELAY); // TODO - maybe fail with stack trace
    } else {
        if(config_display() != ESP_OK) {
            ESP_LOGE(TAG_DISPLAY,"Could not configure display...disactivating\r\n");
            hwcap.display = false;
            if(hwcap.led) {
                led.setLed(1, red);
                vTaskDelay(pdMS_TO_TICKS(500));  //Let us enjoy the led 500ms
            }
        } else {
            hwcap.display = true;

            // Clear the display to remove artifacts that may be in buffer
            lcd.clear(lcd.bounds());

            // Display a text version of the boot screen
            //
            // TODO do it
            const font &font = Bm437_Acer_VGA_8x8_FON;
            const char *bcd_bootscreen = 
                "12345678901234567890\r\n"
                "\r\n"
                "\r\n"
                "\r\n"
                "\r\n"
                "\r\n"
                "\r\n"
                "\r\n"
                "\r\n"
                "\r\n"
                "\r\n"
                "\r\n"
                "\r\n"
                "\r\n"
                "12345678901234567890";
                draw::text(lcd,
                lcd.bounds(),
                bcd_bootscreen,
                font,
                lcd_color::steel_blue);  

                if(hwcap.led) {
                    led.setLed(1, green);
                    vTaskDelay(pdMS_TO_TICKS(500));  //Let us enjoy the led 500ms
                }
        }

        // If the led initialisation failed in the previous step show it on the
        // display
        if(led_error != LED_OK) {
            const font &font = Bm437_Acer_VGA_8x8_FON;
            const char* text_led_error 
                = "      WARNING!      \r\n LED setup failure! \r\nFunctionality impact";
            srect16 text_led_error_rect = font.measure_text((ssize16)lcd.dimensions(),
                                    text_led_error).bounds();
            draw::text(lcd,
                    text_led_error_rect.center((srect16)lcd.bounds()),
                    text_led_error,
                    font,
                    lcd_color::orange_red);
        } 
#endif //CONFIG_DISPLAY_SUPPORT
    }


    // 2. Setup the filesystems 
    //
    // Second we setup the filesystem to enable the boot screen displaying the
    // logo. As we are on it, we fully initilise all filesystems. Delay is not
    // noticable and doing all here is easier to read the code.

    // NVS
    //
    // Used for WiFi and to store simple configuration parameters. If we cannot
    // initialise NVS due to a version change or absence of free pages, we 
    // erase NVS (efectively setting the badge back to factory default) and try
    // again. If after this NVS initialisation is not successfull, we fail
    // startup.
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        err = nvs_flash_init();
    }
    if(err != ESP_OK) {
        ESP_LOGE(TAG_FS, "Could not initialise NVS: %d", err);
#ifdef CONFIG_DISPLAY_SUPPORT
        if(hwcap.display) {
            const font &font = Bm437_Acer_VGA_8x8_FON;
            const char* text_nvs_error = "FATAL ERROR\r\nNVS failed!\r\n";
            srect16 text_nvs_error_rect = font.measure_text((ssize16)lcd.dimensions(),
                                    text_nvs_error).bounds();
            draw::text(lcd,
                    text_nvs_error_rect.center((srect16)lcd.bounds()),
                    text_nvs_error,
                    font,
                    lcd_color::orange_red);  
        }
#endif //CONFIG_DISPLAY_SUPPORT
        led.setLeds(led_fatal_error);
        ESP_ERROR_CHECK(err);                           // Fail if we canot mount nvs.

        //TODO - deep sleep
    }

    // SPIFFS
    //
    // Used to store data, such as for example images to display etc...
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false
    };         
    esp_err_t fs_error = esp_vfs_spiffs_register(&conf);
    if (fs_error != ESP_OK) {
        hwcap.fs_spiffs = false;                 // Mark spiffs as not available
        switch(fs_error) {
            case ESP_FAIL:
                ESP_LOGE(TAG_FS, "Failed to mount or format filesystem");
                break;

            case ESP_ERR_NOT_FOUND:
                ESP_LOGE(TAG_FS, "Failed to find SPIFFS partition");
                break;

            default:
                ESP_LOGE(TAG_FS, "Failed to initialize SPIFFS (%d)", fs_error);
        }
#ifdef CONFIG_DISPLAY_SUPPORT
        if(hwcap.display) {
            const font &font = Bm437_Acer_VGA_8x8_FON;
            const char* text_spiffs_error 
                = "      WARNING!      \r\n   SPIFFS failure   \r\nFunctionality impact\r\n\r\n";
            srect16 text_spiffs_error_rect = font.measure_text((ssize16)lcd.dimensions(),
                                    text_spiffs_error).bounds();
            draw::text(lcd,
                    text_spiffs_error_rect.center((srect16)lcd.bounds()),
                    text_spiffs_error,
                    font,
                    lcd_color::yellow);
        }  
#endif //CONFIG_DISPLAY_SUPPORT
        if(hwcap.led) {
            led.setLed(2, red);
            vTaskDelay(pdMS_TO_TICKS(500));  //Let us enjoy the led 500ms
        }
    } else {
        size_t total = 0, used = 0;
        fs_error = esp_spiffs_info(NULL, &total, &used);
        if (fs_error != ESP_OK) {
            ESP_LOGE(TAG_FS, "Failed to get SPIFFS partition information");
        } else {
            ESP_LOGI(TAG_FS, "Partition size: total: %d, used: %d", total, used);
        }
        if(hwcap.led) {
            led.setLed(2, green);
            vTaskDelay(pdMS_TO_TICKS(500));  //Let us enjoy the led 500ms
        }
    }

    // 3. Setup controller (push buttons)
#ifdef CONFIG_INPUT_SUPPORT
    controller_err_t controller_error = controller.config(); 
    if(controller_error != CONTROLLER_OK) {
        ESP_LOGE(TAG_CONTROLLER, "Could not configure embedded controller. Running without support.");
        hwcap.controller = false;

        if(hwcap.led) {
            led.setLed(3, red);
            vTaskDelay(pdMS_TO_TICKS(500));  //Let us enjoy the led 500ms
        }
#else //CONFIG_INPUT_SUPPORT
    hwcap.controller = false;
#endif //CONFIG_INPUT_SUPPORT

#ifdef CONFIG_DISPLAY_SUPPORT
        if(hwcap.display) {
            const font &font = Bm437_Acer_VGA_8x8_FON;
            const char* text_spiffs_error 
                = "      WARNING!      \r\n   Controller failure   \r\nFunctionality impact\r\n\r\n";
            srect16 text_spiffs_error_rect = font.measure_text((ssize16)lcd.dimensions(),
                                    text_spiffs_error).bounds();
            draw::text(lcd,
                    text_spiffs_error_rect.center((srect16)lcd.bounds()),
                    text_spiffs_error,
                    font,
                    lcd_color::yellow);  
        }
#endif //CONFIG_DISPLAY_SUPPORT

    } else {
        if(hwcap.led) {
            led.setLed(3, green);
            vTaskDelay(pdMS_TO_TICKS(500));  //Let us enjoy the led 500ms
        }
    }


    // 4. Setup UART and the console
    // 
    // After this, we need to check the led scheduler as theoretically (althoug
    // rather unlikely) from the console task leds could be used.

    /* New code not working */
    console_err_t console_error = console.start();
    if(console_error != CONSOLE_OK) {
        ESP_LOGE(TAG_CONSOLE, "Console controller in error state (E:0x%x): Resetting...", console_error);
        console_error = console.start();
    } else {
        // Register WiFi commands
        console.registerCommand("lsap", &lsap, "List available access points.");
        console.registerCommand("wificfg", &wificfg, "Configure wifi.");
        console.registerCommand("apinfo", &apinfo, "Show information about connected access point.");
        
        // System commands
        console.registerCommand("version", &get_version, "Get version of chip and sdk.");
        console.registerCommand("restart", &restart, "Software reset of the chip.");
        console.registerCommand("free", &free_mem, "Get currently free heap memory.");
        console.registerCommand("minheap", &heap_size, "Get minimum size of free heap memory that was available during programm execution.");
        console.registerCommand("tasks", &tasks_info, "Get information about running tasks.");
        console.registerCommand("taskstats", &stats_info, "Get statistics about running tasks.");
        console.registerCommand("intstats", &interval_stats_info, "Get statistics about running tasks during interval.");
    }

    if(console_error != ESP_OK) {
    // Something went wrong spawning the console.Disable capability and 
    // indicate error
    ESP_LOGW(TAG_CONSOLE, "Failed to initialise uart.");
    hwcap.console = false;

    if(hwcap.led) {
        led.setLed(4, red);
        vTaskDelay(pdMS_TO_TICKS(500));  //Let us enjoy the led 500ms
    }

#ifdef CONFIG_DISPLAY_SUPPORT
        if(hwcap.display) {
            const font &font = Bm437_Acer_VGA_8x8_FON;
            const char* text_spiffs_error 
                = "      WARNING!      \r\n   Console failure   \r\nFunctionality impact\r\n\r\n";
            srect16 text_spiffs_error_rect = font.measure_text((ssize16)lcd.dimensions(),
                                    text_spiffs_error).bounds();
            draw::text(lcd,
                    text_spiffs_error_rect.center((srect16)lcd.bounds()),
                    text_spiffs_error,
                    font,
                    lcd_color::yellow);  
        }
#endif //CONFIG_DISPLAY_SUPPORT
    } else {
        if(hwcap.led) {
            led.setLed(4, green);
            vTaskDelay(pdMS_TO_TICKS(500));  //Let us enjoy the led 500ms
        }
    }

    // Start the main event loop
    esp_event_loop_create_default();
    
    // 5. Initialise WiFi (do not yet connect)
    Wifi.setSsid(WIFI_SSID);
    Wifi.setPassword(WIFI_PASS);
    if(Wifi.init() != ESP_OK) {
        ESP_LOGE(TAG_WIFI, "Failed to initialise WiFi");

#ifdef CONFIG_DISPLAY_SUPPORT
        const font &font = Bm437_Acer_VGA_8x8_FON;
        const char* text_spiffs_error 
            = "      WARNING!      \r\n   WiFi failure   \r\nFunctionality impact\r\n\r\n";
        srect16 text_spiffs_error_rect = font.measure_text((ssize16)lcd.dimensions(),
                                text_spiffs_error).bounds();
        draw::text(lcd,
                text_spiffs_error_rect.center((srect16)lcd.bounds()),
                text_spiffs_error,
                font,
                lcd_color::yellow);  
#endif //CONFIG_DISPLAY_SUPPORT

        hwcap.wlan = false;
        if(hwcap.led) {
            led.setLed(5, red);
            vTaskDelay(pdMS_TO_TICKS(500));  //Let us enjoy the led 500ms
        }

    } else {
        // TODO check if leds are occupied / implement locking mechanism
        if(hwcap.led) {
            led.setLed(5, green);
            vTaskDelay(pdMS_TO_TICKS(500));  //Let us enjoy the led 500ms
        }
    }
    
    // Wait to ensure status leds can be read
    vTaskDelay(pdMS_TO_TICKS(1500));

#ifdef CONFIG_DEBUG_STACK
    /* Inspect our own high water mark on entering the task. */
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    ESP_LOGD(TAG_STACK, "Main:setup(): High watermark for stack at end is: %d", uxHighWaterMark);
#endif
}


/****************************************************************
 * Main
 * 
 * The main application.
 * 
 * TODO:
 *  - Proper error checking
 ****************************************************************/
void app_main(void) {

#ifdef CONFIG_DEBUG_HEAP
    // Initialise heap tracing to make it available for use later.
    ESP_ERROR_CHECK( heap_trace_init_standalone(trace_record, NUM_RECORDS) );
#endif //CONFIG_DEBUG_HEAP

    // TODO - make logging for event loop configurable
    // esp_log_level_set("event", WIFI_LOG_LEVEL);

    App.setup();
    App.run();
}
