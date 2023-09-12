#include "main.hpp"
#include "led_patterns/led_patterns.h"


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
    // NOTE: To use the framework, add console commands and modules. Of course, 
    //       you can also customize the whole main loop and do not need to use 
    //       the provided one.


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
        mc.cursor->addEntry(mc.createActionItem("Demo Mode", modDemoMode::demoMode<lcd_type>, &lcd));

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
 */
void Main::setup(void) {
#ifdef CONFIG_DEBUG_STACK
    UBaseType_t uxHighWaterMark;

    /* Inspect our own high water mark on entering the task. */
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    ESP_LOGD(TAG_STACK, "Main:setup(): High watermark for stack at start is: %d", uxHighWaterMark);
#endif
    // Indicates which driver cluster we are initialising
    uint8_t item_number = 0;

    // LED colors we us throughout the routing
    rgbLEDPixel critical_fail = {
        .red = 0x0F,
        .green = 0x00,
        .blue = 0x00
    };

    rgbLEDPixel fail = {
        .red = 0x0F,
        .green = 0x0F,
        .blue = 0x00
    };

    rgbLEDPixel ok = {
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
        led_fatal_error.led[i] = critical_fail;      
    }

    // Bootup messages for display
    const font &font = Bm437_Acer_VGA_8x8_FON;
    const char *bootup_title = "BCD-0o27 booting...";
    const char *bootup_led_msg = "Init LEDs";
    const char *bootup_display_msg = "Init display";
    const char *bootup_fs_msg = "Init filesys";
    const char *bootup_controller_msg = "Init controller";
    const char *bootup_uart_msg = "Init UART";
    const char *bootup_wifi_msg = "Init wifi";
    const char *fail_msg = "FAIL";
    const char *ok_msg = "  OK";

    // Areas on boot screen
    rect16 bootup_title_area(0,0,lcd.width, 2 * font.height());
    rect16 bootup_msg_area(0, bootup_title_area.y2, 
        lcd.width - font.average_width() * 5, bootup_title_area.y2 + font.height());
    rect16 bootup_status_area(bootup_msg_area.x2 + font.average_width(), 
        bootup_msg_area.y1, lcd.width, bootup_msg_area.y2);


    // 0. Setup LEDs
    //
    // First we try to set up the leds. (We will use the leds to display bootup
    // status.)
    item_number = 0;
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
        led_error = led.setLed(0, ok);
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
    item_number++;
    if(!spi_host.initialized()) {
        ESP_LOGE(TAG_DISPLAY, "SPI host initialization error. Halting...\r\n");
        if(hwcap.led) {
            led_fatal_error.led[item_number] = clear;                           // Clear the led to indicate component
            led.setLeds(led_fatal_error);
        }
        ESP_ERROR_CHECK(ESP_FAIL);                                              // Fail with stack trace
    } else {
        
        hwcap.display = true;

        // Clear the display to remove artifacts that may be in buffer
        lcd.clear(lcd.bounds());

        // Display a text version of the boot screen
        //
        draw::text(lcd, bootup_title_area, bootup_title, font, 
            lcd_color::steel_blue);
        draw::text(lcd, bootup_msg_area, bootup_led_msg, font, 
            lcd_color::steel_blue);
        if(led_error == LED_OK) {
            draw::text(lcd, bootup_status_area, ok_msg, font, 
                lcd_color::green);
        } else {
            draw::text(lcd, bootup_status_area, fail_msg, font, 
                lcd_color::yellow);
        }
        draw::text(lcd, 
            bootup_msg_area.offset(0,item_number * font.height()), 
            bootup_display_msg, font, lcd_color::steel_blue);
        draw::text(lcd, 
            bootup_status_area.offset(0, item_number * font.height()), 
            ok_msg, font, lcd_color::green);

        // Set the status led of the display to indicate success
        if(hwcap.led) {
            led.setLed(item_number, ok);
            vTaskDelay(pdMS_TO_TICKS(500));                                 //Let us enjoy the led 500ms
        }
        
    }
#endif //CONFIG_DISPLAY_SUPPORT


    // 2. Setup the filesystems 
    //
    // Second we setup the filesystem to enable the boot screen displaying the
    // logo. As we are on it, we fully initilise all filesystems. Delay is not
    // noticable and doing all here is easier to read the code.
    item_number++;

    // NVS
    //
    // Used for WiFi and to store simple configuration parameters. If we cannot
    // initialise NVS due to a version change or absence of free pages, we 
    // erase NVS (efectively setting the badge back to factory default) and try
    // again. If after this NVS initialisation is not successfull, we fail
    // startup.
#ifdef CONFIG_DISPLAY_SUPPORT
    draw::text(lcd, 
        bootup_msg_area.offset(0,item_number * font.height()), 
        bootup_fs_msg, font, 
        lcd_color::steel_blue);
#endif //CONFIG_DISPLAY_SUPPORT

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        err = nvs_flash_init();
    }
    if(err != ESP_OK) {
        ESP_LOGE(TAG_FS, "Could not initialise NVS: %s (%d)", 
            esp_err_to_name(err), err);
#ifdef CONFIG_DISPLAY_SUPPORT
        if(hwcap.display) {
            draw::text(lcd, 
                bootup_status_area.offset(0, item_number * font.height()),
                fail_msg, 
                font, 
                lcd_color::orange_red); 
        }
#endif //CONFIG_DISPLAY_SUPPORT
        if(hwcap.led) {
            led_fatal_error.led[item_number] = clear;
            led.setLeds(led_fatal_error);
        }
        ESP_ERROR_CHECK(err);                                                   // Fail if we canot mount nvs.
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
        hwcap.fs_spiffs = false;                                                // Mark spiffs as not available
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
            draw::text(lcd, 
                bootup_status_area.offset(0,item_number * font.height()), 
                fail_msg, font, lcd_color::yellow);
        }  
#endif //CONFIG_DISPLAY_SUPPORT
        if(hwcap.led) {
            led.setLed(item_number, fail);
            vTaskDelay(pdMS_TO_TICKS(500));                                     //Let us enjoy the led 500ms
        }
    } else {
        size_t total = 0, used = 0;
        fs_error = esp_spiffs_info(NULL, &total, &used);
        if (fs_error != ESP_OK) {
            ESP_LOGE(TAG_FS, "Failed to get SPIFFS partition information");
        } else {
            ESP_LOGI(TAG_FS, "Partition size: total: %d, used: %d", 
                total, used);
        }
#ifdef CONFIG_DISPLAY_SUPPORT
        if(hwcap.display) {
            draw::text(lcd, 
                bootup_status_area.offset(0,item_number * font.height()), 
                ok_msg, font, lcd_color::green);
        }  
#endif //CONFIG_DISPLAY_SUPPORT
        if(hwcap.led) {
            // Set led to indicate success
            led.setLed(item_number, ok);
            vTaskDelay(pdMS_TO_TICKS(500));                                     //Let us enjoy the led 500ms
        }
    }

    // 3. Setup controller (push buttons)
#ifdef CH405LABS_CONTROLLER_SUPPORT
    item_number++;

#ifdef CONFIG_DISPLAY_SUPPORT
    draw::text(lcd, 
        bootup_msg_area.offset(0,item_number * font.height()), 
        bootup_controller_msg, font, 
        lcd_color::steel_blue);
#endif //CONFIG_DISPLAY_SUPPORT

    controller_err_t controller_error = controller.config(); 
    if(controller_error != CONTROLLER_OK) {
        ESP_LOGE(TAG_CONTROLLER, "Could not configure embedded controller." 
            "Running without support.");
        hwcap.controller = false;
#ifdef CONFIG_DISPLAY_SUPPORT
        if(hwcap.display) {
            draw::text(lcd, bootup_status_area, fail_msg, font, 
                    lcd_color::yellow);
        }  
#endif //CONFIG_DISPLAY_SUPPORT
        if(hwcap.led) {
            led.setLed(item_number, fail);
            vTaskDelay(pdMS_TO_TICKS(500));                                     //Let us enjoy the led 500ms
        }
    } else {
#ifdef CONFIG_DISPLAY_SUPPORT
        if(hwcap.display) {
            draw::text(lcd, 
                bootup_status_area.offset(0,item_number * font.height()), 
                ok_msg, font, lcd_color::green);
        }  
#endif //CONFIG_DISPLAY_SUPPORT
        if(hwcap.led) {
            led.setLed(item_number, ok);
            vTaskDelay(pdMS_TO_TICKS(500));  //Let us enjoy the led 500ms
        }
    }
#else //CH405LABS_CONTROLLER_SUPPORT
    hwcap.controller = false;
#endif //CH405LABS_CONTROLLER_SUPPORT



    // 4. Setup UART and the console
    // 
    // TODO After this, we should check the led scheduler as theoretically 
    // (althoug rather unlikely) from the console task leds could be used.
    item_number++;

#ifdef CONFIG_DISPLAY_SUPPORT
    draw::text(lcd, 
        bootup_msg_area.offset(0,item_number * font.height()), 
        bootup_uart_msg, font, 
        lcd_color::steel_blue);
#endif //CONFIG_DISPLAY_SUPPORT

    // Try to start the console. If this fails, we retry once again.
    console_err_t console_error = console.start();
    if(console_error != CONSOLE_OK) {
        ESP_LOGE(TAG_CONSOLE, "Console controller in error state (E:0x%x):" 
            "Resetting...", console_error);
        console_error = console.start();
    } 
    if(console_error == CONSOLE_OK) {
        hwcap.console = true;
#ifdef CONFIG_DISPLAY_SUPPORT
        if(hwcap.display) {
            draw::text(lcd, 
                bootup_status_area.offset(0,item_number * font.height()), 
                ok_msg, font, lcd_color::green);
        }  
#endif //CONFIG_DISPLAY_SUPPORT
        if(hwcap.led) {
            led.setLed(item_number, ok);
            vTaskDelay(pdMS_TO_TICKS(500));  //Let us enjoy the led 500ms
        }
    } else {
        // Something went wrong spawning the console.Disable capability and 
        // indicate error
        ESP_LOGW(TAG_CONSOLE, "Failed to initialise uart.");
        hwcap.console = false;

#ifdef CONFIG_DISPLAY_SUPPORT
        if(hwcap.display) {
            draw::text(lcd, 
                bootup_status_area.offset(0,item_number * font.height()), 
                fail_msg, font, lcd_color::yellow);
        }  
#endif //CONFIG_DISPLAY_SUPPORT
        if(hwcap.led) {
            led.setLed(item_number, fail);
            vTaskDelay(pdMS_TO_TICKS(500));  //Let us enjoy the led 500ms
        }
    }

    // Start the main event loop
    //
    // We start the main loop here, as all the drivers that need the main loop
    // to not yet be started (or need to be initialised before drivers that 
    // require the event loop not yet being started are done.)
    esp_event_loop_create_default();
    
    // 5. Initialise WiFi (do not yet connect)
    //
    // We prepare the wifi such that it is ready. However, we do not yet connect
    // even if default credentials are provided.
    item_number++;

#ifdef CONFIG_DISPLAY_SUPPORT
    draw::text(lcd, 
        bootup_msg_area.offset(0,item_number * font.height()), 
        bootup_wifi_msg, font, 
        lcd_color::steel_blue);
#endif //CONFIG_DISPLAY_SUPPORT

    Wifi.setSsid(WIFI_SSID);
    Wifi.setPassword(WIFI_PASS);
    if(Wifi.init() != ESP_OK) {
        ESP_LOGE(TAG_WIFI, "Failed to initialise WiFi");
        hwcap.wlan = false;

#ifdef CONFIG_DISPLAY_SUPPORT
        if(hwcap.display) {
            draw::text(lcd, 
                bootup_status_area.offset(0,item_number * font.height()), 
                fail_msg, font, lcd_color::yellow);
        }  
#endif //CONFIG_DISPLAY_SUPPORT
        if(hwcap.led) {
            led.setLed(item_number, fail);
            vTaskDelay(pdMS_TO_TICKS(500));                                     //Let us enjoy the led 500ms
        }

    } else {
        // TODO check if leds are occupied / implement locking mechanism. 
        // Console task or main loop might use them (unlikely)
#ifdef CONFIG_DISPLAY_SUPPORT
        if(hwcap.display) {
            draw::text(lcd, 
                bootup_status_area.offset(0,item_number * font.height()), 
                ok_msg, font, lcd_color::green);
        }  
#endif //CONFIG_DISPLAY_SUPPORT
        if(hwcap.led) {
            led.setLed(item_number, ok);
            vTaskDelay(pdMS_TO_TICKS(500));  //Let us enjoy the led 500ms
        }
    }
    
    // Give the user some time to read boot screen - if present - and status 
    // leds.
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

    App.setup();
    App.run();
}
