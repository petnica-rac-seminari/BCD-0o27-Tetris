#ifndef BCD_MODULE_SAO_DEMO_HPP
#define BCD_MODULE_SAO_DEMO_HPP

#pragma once

#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "../../helpers/debug.hpp"

#define SAO_SCL         (gpio_num_t)CONFIG_SAO_SCL
#define SAO_SDA         (gpio_num_t)CONFIG_SAO_SDA
#define SAO_PIN0        (gpio_num_t)CONFIG_SAO_PIN0
#define SAO_PIN1        (gpio_num_t)CONFIG_SAO_PIN1

template<typename Destination>
int saoBlink(void *param) {
    
    // Configure the GPIOs
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = ((1ULL<<CONFIG_SAO_PIN0) 
                                | (1ULL<<CONFIG_SAO_PIN1)
                                | (1ULL<<CONFIG_SAO_SCL)
                                | (1ULL<<CONFIG_SAO_SDA));
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;

    esp_err_t error = gpio_config(&io_conf);
    if(error != ESP_OK) {
        return -1;
    }
  

    gpio_set_level(SAO_PIN0, 0);
    gpio_set_level(SAO_PIN1, 0);
    gpio_set_level(SAO_SDA, 0);
    gpio_set_level(SAO_SCL, 0);

    // Blink 10 times
    uint32_t on = 1;
    for(int i = 0; i < 20; i++) {
        gpio_set_level(SAO_PIN0, on);
        gpio_set_level(SAO_PIN1, !on);
        gpio_set_level(SAO_SDA, !on);
        gpio_set_level(SAO_SCL, !on);
        vTaskDelay(pdMS_TO_TICKS(50));

        gpio_set_level(SAO_PIN0, !on);
        gpio_set_level(SAO_PIN1, on);
        gpio_set_level(SAO_SDA, !on);
        gpio_set_level(SAO_SCL, !on);
        vTaskDelay(pdMS_TO_TICKS(50));

        gpio_set_level(SAO_PIN0, !on);
        gpio_set_level(SAO_PIN1, !on);
        gpio_set_level(SAO_SDA, on);
        gpio_set_level(SAO_SCL, !on);
        vTaskDelay(pdMS_TO_TICKS(50));

        gpio_set_level(SAO_PIN0, !on);
        gpio_set_level(SAO_PIN1, !on);
        gpio_set_level(SAO_SDA, !on);
        gpio_set_level(SAO_SCL, on);
        vTaskDelay(pdMS_TO_TICKS(50));

        gpio_set_level(SAO_PIN0, !on);
        gpio_set_level(SAO_PIN1, !on);
        gpio_set_level(SAO_SDA, on);
        gpio_set_level(SAO_SCL, !on);
        vTaskDelay(pdMS_TO_TICKS(50));

        gpio_set_level(SAO_PIN0, !on);
        gpio_set_level(SAO_PIN1, on);
        gpio_set_level(SAO_SDA, !on);
        gpio_set_level(SAO_SCL, !on);
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    gpio_set_level(SAO_PIN0, 0);
    gpio_set_level(SAO_PIN1, 0);
    gpio_set_level(SAO_SDA, 0);
    gpio_set_level(SAO_SCL, 0);
    
    return 0;
}
#endif // BCD_MODULE_SAO_DEMO_HPP