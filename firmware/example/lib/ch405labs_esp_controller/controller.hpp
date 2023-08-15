/** 
 * @file        controller.hpp
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
 * @bug No known bugs.
 * 
 * @todo Implement an interrupt mode that allows to register a handler for button
 *          presses. Interrupt mode should be switchable
 * 
 * @todo Implement a task that allows polling the cursor state in regular 
 *          intervals. Multiple task should be easily spawnable. The owner that 
 *          spawns the task should be able to destroy it. The owner should also 
 *          be allowed to set the polling intervall. There must be a possibility 
 *          to store the different button presses in different modes (eg. queue 
 *          of all presses (with timestam?), last press only (maybe does not 
 *          make much sense) and first press only);
 */

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/gpio.h"


////////////////////////////////////////////////////////////////////////////////
// Debugging
////////////////////////////////////////////////////////////////////////////////
static const char TAG_CONTROLLER[] = CONFIG_TAG_INPUT;                          /**< TAG for ESP_LOGX macro. */


////////////////////////////////////////////////////////////////////////////////
// Error handling
////////////////////////////////////////////////////////////////////////////////

/** @typedef The error type for any input related error */
typedef int controller_err_t;

#define CONTROLLER_FAIL                 -1                                      /**< Generic failure */
#define CONTROLLER_OK                   0x000                                   /**< Success */
#define CONTROLLER_ALREADY_CONFIGURED   0x001                                   /**< Controller already configured */
#define CONTROLLER_GPIO_ERROR           0x010                                   /**< Generic GPIO error */
#define CONTROLLER_GPIO_ERROR_CONFIG    0x011                                   /**< GPIO configuration faiiled */
#define CONTROLLER_NOT_CONFIGURED       0x012                                   /**< Controller not yet configured */
#define CONTROLLER_TIMEOUT              0x107                                   /**< Could not read state before timeout */


////////////////////////////////////////////////////////////////////////////////
// Button mapping
////////////////////////////////////////////////////////////////////////////////

/**
 * @typedef for identifying buttons (yes, probably the worst description ever)
 * 
 */
typedef uint8_t buttonIdentifier;

#define BUTTON_UP       ((buttonIdentifier)0b00000100)
#define BUTTON_DOWN     ((buttonIdentifier)0b00000010)
#define BUTTON_LEFT     ((buttonIdentifier)0b00000001)
#define BUTTON_RIGHT    ((buttonIdentifier)0b00001000)

#define BUTTON_A        ((buttonIdentifier)0b01000000)
#define BUTTON_B        ((buttonIdentifier)0b10000000)
#define BUTTON_X        ((buttonIdentifier)0b00010000)
#define BUTTON_Y        ((buttonIdentifier)0b00100000)


////////////////////////////////////////////////////////////////////////////////
// Driver class
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Driver class for BCD on board inputs
 * 
 * This class provides functionality to configure and deal with push button
 * inputs on the BalcCon Cyberdeck.
 */
class controllerDriver {
    private:
        static const gpio_num_t DATA_PIN = (gpio_num_t) CONFIG_INPUT_PIN_DATA;
        static const gpio_num_t CLOCK_PIN = (gpio_num_t) CONFIG_INPUT_PIN_CLK;
        static const gpio_num_t LOAD_PIN = (gpio_num_t) CONFIG_INPUT_PIN_LOAD;
        static const gpio_num_t CLOCK_ENABLE_PIN 
            = (gpio_num_t) CONFIG_INPUT_PIN_CLE;
        static const unsigned long long GPIO_OUTPUT_PIN_SEL = 
                ((1ULL<<CLOCK_ENABLE_PIN) | (1ULL<<CLOCK_PIN) 
                | (1ULL<<LOAD_PIN));
        static const unsigned long long GPIO_INPUT_PIN_SEL = (1ULL<<DATA_PIN);

        static bool _configured;
        static esp_err_t _error_state;

        int _dataIn = 0;


    public:
        /** @brief Configure the peripherals
         *
         *  TODO.
         *
         *  TODO @param input_conf the configuration of the inputs
         *  @return ESP_OK if success or error code otherwise
         */
        static esp_err_t config();

        /**
         * @brief Get error state of the controller
         * 
         * This function allows checking if the controller is function properly.
         * 
         * @return esp_err_t The error code of the last error
         */
        static esp_err_t getErrorState();


        /**
         * @brief Capture the input states of the controller buttons
         * 
         */
        controller_err_t capture();

        /**
         * @brief Sample the input states of the controller
         * 
         * Sampling reads the controller button state. It adds any button
         * pressed to the previously recorded button state. For example, if our 
         * sample already contains the up and left button and we sample again 
         * while the A button is pressed, the up, left and A button are now 
         * recorded as pressed. This allows to query the controller multiple 
         * times within an interval and record all buttons in this interval, 
         * even if not pressed at exactly the same time.
         * Sampling adds also to a previous capture. However, doing another 
         * capture resets the sample.
         * 
         * @return controller_err_t 
         */
        controller_err_t sample();


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
        bool getButtonState(buttonIdentifier id);

        /**
         * @brief Reset all button presses to not pressed
         * 
         * @return Always ESP_OK
         */
        controller_err_t clear();
        
    
        /** @brief Diagnose the input peripherals
         *
         *  Queries the user over serial to press the buttons in a
         *  certain manner. If the recorded button presses match the
         *  requested pattern the test is successful, otherwise it
         *  failes. 
         * 
         *  Buttons must be pressed within a timeframe. If the timeframe
         *  passess and the right button are not pressed and held the
         *  test fails.
         *
         *  TODO @param input_conf the configuration of the inputs
         *  @return true if success, false otherwise
         */
        controller_err_t diagnose();
};