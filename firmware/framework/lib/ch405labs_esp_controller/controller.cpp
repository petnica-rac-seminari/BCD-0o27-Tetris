
/** 
 * @file        controller.cpp
 * @author      Florian Schütz (fschuetz@ieee.org)
 * @brief       Driver implementation to handle input peripherals.
 * @version     0.9
 * @date        2022-10-14
 * @copyright   Copyright (c) 2022, Florian Schuetz, released under MIT license
 *  
 * Prototypes for the input peripheral driver and eventually any macros, 
 * constants, or global variables to handle user interactions using the 
 * BCD-0o26's main peripherals.
 *
 *  @bug No known bugs.
 */

#include "controller.hpp"


bool controllerDriver::_configured = false;
controller_err_t controllerDriver::_error_state = CONTROLLER_NOT_CONFIGURED;

/****************************************************************
 * Config Inputs
 * 
 * Configure the GPIO pins for the input buttons.
 * 
 * TODO:
 *  - Proper error checking
 ****************************************************************/
controller_err_t controllerDriver::config() {

    if(!_configured) {

        // Variable for error tracking
        esp_err_t error = CONTROLLER_OK;

        // Configure output pins (CLE, CLK, PL)
        gpio_config_t io_conf;
        io_conf.intr_type = GPIO_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;

        
        error = gpio_config(&io_conf);
        if(error != ESP_OK) {
            _error_state = CONTROLLER_GPIO_ERROR_CONFIG;
            return CONTROLLER_GPIO_ERROR_CONFIG;
        }

        // Configure input pin (SA).
        // Reuse parameters from above.
        io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
        io_conf.mode = GPIO_MODE_INPUT;
        error = gpio_config(&io_conf);

        if(error != ESP_OK) {
            _error_state = CONTROLLER_GPIO_ERROR_CONFIG;
            return CONTROLLER_GPIO_ERROR_CONFIG;
        }

        _configured = true;
        _error_state = CONTROLLER_OK;
        return CONTROLLER_OK;

    } else {
        _error_state = CONTROLLER_OK;
        return CONTROLLER_ALREADY_CONFIGURED;
    }
}

 /**
  * @brief Get error state of the controller
  * 
  * This function allows checking if the controller is function properly.
  * 
  * @return esp_err_t The error code of the last error
  */
controller_err_t controllerDriver::getErrorState() {
    return _error_state;
}

/**
 * @brief Capture the input states of the controller buttons
 * 
 */
controller_err_t controllerDriver::capture() {

    if(!_configured) {
        return CONTROLLER_NOT_CONFIGURED;
    }

    int value = 0;                          // Store single button push
    _dataIn = 0;                             // Reset data in

#ifdef CONFIG_DEBUG_STACK
    UBaseType_t uxHighWaterMark;

    /* Inspect our own high water mark on entering the task. */
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    ESP_LOGD(CONFIG_TAG_STACK, "controllerDriver::capture(): High watermark for stack at start is: %d", uxHighWaterMark);
#endif

    // Prepare the clock by disabling it (avoid shift) and then
    // setting the clock pin to high
    gpio_set_level(CLOCK_ENABLE_PIN, 1);
    vTaskDelay(5 / portTICK_PERIOD_MS); // TODO - check if needed
    gpio_set_level(CLOCK_PIN, 1);
    

    // Load button state in register by pulsing the parallel load pin
    gpio_set_level(LOAD_PIN, 0);
    gpio_set_level(LOAD_PIN, 1);

    // Enable the clock and prepare a cycle by setting the clock pin
    // to low. Note: The clock is active low.
    gpio_set_level(CLOCK_ENABLE_PIN, 0);
    gpio_set_level(CLOCK_PIN, 0);

    // Query each button by cycling through all of the values in the
    // shift register. After reading out a button value, the next one
    // is loaded by pulsing the clock from low to high (and then reset
    // to low to prepare next run)
    for(int i = 0; i < 8; i++) {
        value = gpio_get_level(DATA_PIN);   // TODO maybe directly _dataIn = _dataIn | (gpio_get_level(DATA_PIN)<<i), though slower when 0

        if(value) {
            _dataIn = _dataIn | (int)(1<<i);
        }
        gpio_set_level(CLOCK_PIN, 1);
        gpio_set_level(CLOCK_PIN, 0);
    }

    // Disable the clock
    gpio_set_level(CLOCK_ENABLE_PIN, 1);

    ESP_LOGD(TAG_CONTROLLER, "controllerDriver state captured: %d", _dataIn);

#ifdef CONFIG_DEBUG_STACK
    /* Inspect our own high water mark on entering the task. */
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    ESP_LOGD(CONFIG_TAG_STACK, "controllerDriver::capture(): High watermark for stack at end is: %d", uxHighWaterMark);
#endif

    return CONTROLLER_OK;
}

/**
 * @brief 
 * 
 *
 * @return 
 */
controller_err_t controllerDriver::sample() {
#ifdef CONFIG_DEBUG_STACK
    UBaseType_t uxHighWaterMark;

    /* Inspect our own high water mark on entering the task. */
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    ESP_LOGD(CONFIG_TAG_STACK, "controllerDriver::sample(): High watermark for stack at start is: %d", uxHighWaterMark);
#endif

    // Make a backup of the old controller state
    int old_dataIn = _dataIn;

    // Capture the controller state
    controller_err_t err = this->capture();
    if( err != CONTROLLER_OK) {
        ESP_LOGE(TAG_CONTROLLER, "Could not sample. Restoring old state.");
        _dataIn = old_dataIn;
        return err;
    }

    // Or the two states together.
    _dataIn |= old_dataIn;

#ifdef CONFIG_DEBUG_STACK
    /* Inspect our own high water mark on entering the task. */
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    ESP_LOGD(CONFIG_TAG_STACK, "controllerDriver::sample(): High watermark for stack at end is: %d", uxHighWaterMark);
#endif

    return CONTROLLER_OK;

}


/**
 * @brief Get the state of a single button or a combination of buttons 
 *         
 * Returns true if all the buttons specified in the argument are pressed.
 * To query multiple buttons use | to combine them (eg. BUTTON_A | BUTTON_B).
 * 
 * @param id the identifier of the button to query (or combination)
 * @return true if the button (combination) is pressed
 * @return false otherwise
 */
bool controllerDriver::getButtonState(buttonIdentifier id) {
    return (bool)((buttonIdentifier)_dataIn & id);
}

/**
 * @brief Reset all button presses to not pressed
 * 
 * @return Always ESP_OK
 */
controller_err_t controllerDriver::clear() {
    _dataIn = 0;
    return CONTROLLER_OK;
}

/****************************************************************
    Input Test
    -------------------------------------------------------------
    Test all the button inputs.
*****************************************************************/
controller_err_t controllerDriver::diagnose() {

    if(!_configured) {
        return CONTROLLER_NOT_CONFIGURED;
    }

    // Prepare the clock by disabling it (avoid shift) and then
    // setting the clock pin to high
    gpio_set_level(CLOCK_ENABLE_PIN, 1);
    vTaskDelay(5 / portTICK_PERIOD_MS);
    gpio_set_level(CLOCK_PIN, 1);

    ///////////////////////////////////////////////////////////////////
    // Button 1,2,3,4
    ///////////////////////////////////////////////////////////////////

    // Ask user to press button one, two, three and four before timout.
    printf("Press and hold button one, two, three and four...");
    for (int i = 5; i >= 0; i--) {
        printf("%d, ", i);
        fflush(stdout);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    // Gather button pushes
    int value = 0;                          // Store single button push
    int dataIn = 0;                         // Records all button states

    // Load button state in register by pulsing the parallel load pin
    gpio_set_level(LOAD_PIN, 0);
    gpio_set_level(LOAD_PIN, 1);

    // Enable the clock and prepare a cycle by setting the clock pin
    // to low. Note: The clock is active low.
    gpio_set_level(CLOCK_ENABLE_PIN, 0);
    gpio_set_level(CLOCK_PIN, 0);

    // Query each button by cycling through all of the values in the
    // shift register. After reading out a button value, the next one
    // is loaded by pulsing the clock from low to high (and then reset
    // to low to prepare next run)
    for(int i = 0; i < 8; i++) {
        value = gpio_get_level(DATA_PIN);

        if(value) {
            int a = (1<<i);
            dataIn = dataIn | a;
        }
        gpio_set_level(CLOCK_PIN, 1);
        gpio_set_level(CLOCK_PIN, 0);
    }

    // Disable the clock
    gpio_set_level(CLOCK_ENABLE_PIN, 1);

    // Check if button pushes were recognised
    if(dataIn == 0b00001111) {
        printf(" OK\n");
    } else {
        printf(" NOT OK (was %d)", dataIn);
        return CONTROLLER_FAIL;
    }
    
    ///////////////////////////////////////////////////////////////////
    // Button 5,6,7,8
    ///////////////////////////////////////////////////////////////////

    // Reset dataIn
    dataIn = 0;

    // Ask user to press button five, six, seven and eight before timout.
    printf("Press and hold button five, six, seven and eight...");
    for (int i = 5; i >= 0; i--) {
        printf("%d, ", i);
        fflush(stdout);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    // Load button state in register by pulsing the parallel load pin
    gpio_set_level(LOAD_PIN, 0);
    gpio_set_level(LOAD_PIN, 1);

    // Enable the clock and prepare a cycle by setting the clock pin
    // to low. Note: The clock is active low.
    gpio_set_level(CLOCK_ENABLE_PIN, 0);
    gpio_set_level(CLOCK_PIN, 0);

    // Query each button by cycling through all of the values in the
    // shift register. After reading out a button value, the next one
    // is loaded by pulsing the clock from low to high (and then reset
    // to low to prepare next run)
    for(int i = 0; i < 8; i++) {
        value = gpio_get_level(DATA_PIN);

        if(value) {
            int a = (1<<i);
            dataIn = dataIn | a;
        }
        gpio_set_level(CLOCK_PIN, 1);
        gpio_set_level(CLOCK_PIN, 0);
    }

    // Disable the clock
    gpio_set_level(CLOCK_ENABLE_PIN, 1);

    // Check if button presses were recognised
    if(dataIn == 0b11110000) {
        printf(" OK\n");
    } else {
        printf(" NOT OK (was %d)", dataIn);
        return CONTROLLER_FAIL;
    }

    ///////////////////////////////////////////////////////////////////
    // No button pressed
    ///////////////////////////////////////////////////////////////////

    // Reset dataIn
    dataIn = 0;

    // Ask user to release all buttons before timout.
    printf("Release all buttons...");
    for (int i = 5; i >= 0; i--) {
        printf("%d, ", i);
        fflush(stdout);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    // Load button state in register by pulsing the parallel load pin
    gpio_set_level(LOAD_PIN, 0);
    gpio_set_level(LOAD_PIN, 1);

    // Enable the clock and prepare a cycle by setting the clock pin
    // to low. Note: The clock is active low.
    gpio_set_level(CLOCK_ENABLE_PIN, 0);
    gpio_set_level(CLOCK_PIN, 0);

    // Query each button by cycling through all of the values in the
    // shift register. After reading out a button value, the next one
    // is loaded by pulsing the clock from low to high (and then reset
    // to low to prepare next run)
    for(int i = 0; i < 8; i++) {
        value = gpio_get_level(DATA_PIN);

        if(value) {
            int a = (1<<i);
            dataIn = dataIn | a;
        }
        gpio_set_level(CLOCK_PIN, 1);
        gpio_set_level(CLOCK_PIN, 0);
    }

    // Disable the clock
    gpio_set_level(CLOCK_ENABLE_PIN, 1);

    // Check if all buttons were released
    if(dataIn == 0b00000000) {
        printf(" OK\n");
    } else {
        printf(" NOT OK (was %d)", dataIn);
        return CONTROLLER_FAIL;
    }

    
    return CONTROLLER_OK;
}