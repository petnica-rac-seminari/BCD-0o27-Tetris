idf_component_register(SRC_DIRS        "./src" 
                       INCLUDE_DIRS     "./include"
                       REQUIRES         gfx ch405labs_gfx_drivers 
                                        ch405labs_esp_wifi
                                        ch405labs_esp_controller
                                        ch405labs_esp_led
                                        ch405labs_esp_console
                                        spiffs nvs_flash vfs)
