/** 
 * @file        ch405labs_led.cpp
 * @author      Florian Schütz (fschuetz@ieee.org)
 * @brief       Driver and pattern generations functions for driving addressable
 *              leds.
 * @version     0.9
 * @date        2022-08-07
 * @copyright   Copyright (c) 2022, Florian Schuetz, released under MIT license
 *
 * This file provides a c++ driver class to drive adressable LEDs. The driver
 * currently only supports WS2812 leds. 
 * Asides driving the hardware, this library further provides a structure to 
 * schedule the display of patterns. A Pattern is a sequence of led blinkings. 
 * The pattern must be provided as a function. The function must have a pointer 
 * as argument (can be $NULL if the function does not use it). This pointer is 
 * usually used to point to a specific argument or an argument list and cast 
 * accordingly. To ensure consistency across runs this argument pointer is 
 * recorded in the pattern structures. 
 * If a function dynamically allocates memory that remains allocated across 
 * executions of the pattern, a clenaup function must be provided as well in 
 * order to prevent memory leaks. Note that once a pattern is successfully put
 * forward for displaying, all of the datastructures are now owned by the LED
 * pattern display thread. Under no circumstance you shall reuse any 
 * dynamically allocated memory references. The pattern structure itself can be 
 * safely delted or reused, as it was copied when sent to the LED display 
 * thread. For convenience, a pattern generator is provided. This allows the 
 * generation of patterns consisting of a list of states with execution time. 
 * This allows dynamic pattern generation. I the pattern is known at time of 
 * programming then it should be preffered to directly provide a function 
 * rather than using the generator, as this usually is more efficient.
 *
 * @bug FIXED - LEDs (almost) all white when concurrent mode and log level 
 *      warning or when integrating rmt initialisation in code directly in init 
 *      function. Similar to: https://github.com/espressif/esp-idf/issues/5237
 *      Fix: Initialisation of flags in rmt_config_t was missing.
 * 
 * @todo
 *      - Ensure that max bits per channel is 32
 *      - Adapt to new RMT driver
 */

#pragma once

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_log.h>
#include <esp_err.h>
#include <driver/gpio.h>
#include <driver/rmt.h>
#include <mutex>

////////////////////////////////////////////////////////////////////////////////
// Menuconfig options
////////////////////////////////////////////////////////////////////////////////
#define LED_IF_NUM_LED           CONFIG_LED_IF_NUM_LED                          /**< Number of leds to drive */
#define LED_IF_CONTROL_EN_PIN    (gpio_num_t)CONFIG_LED_IF_CONTROL_EN           /**< LED enable GPIO number */
#define LED_IF_RMT_TX_PIN        (gpio_num_t)CONFIG_LED_IF_RMT_TX_GPIO          /**< RMT transmission GPIO number */
#define LED_IF_RMT_TX_CHANNEL	 (rmt_channel_t)CONFIG_LED_IF_RMT_TX_CHANNEL    /**< RMT channel to use */
#define LED_IF_BITS_PER_COLOR    CONFIG_LED_IF_BITS_PER_COLOR_CHANNEL           /**< Bits per color channel used */
#define LED_IF_BITS_RED_CHANNEL  CONFIG_LED_IF_BITS_RED_CHANNEL                 /**< Bits of red hardware channel */
#define LED_IF_BITS_GREEN_CHANNEL CONFIG_LED_IF_BITS_GREEN_CHANNEL              /**< Bits of green hardware channel */
#define LED_IF_BITS_BLUE_CHANNEL CONFIG_LED_IF_BITS_BLUE_CHANNEL                /**< Bits of blue hardware channel */
#define LED_IF_BITS_PER_LED_CMD  (CONFIG_LED_IF_BITS_RED_CHANNEL \
                                    + CONFIG_LED_IF_BITS_GREEN_CHANNEL \
                                    + CONFIG_LED_IF_BITS_BLUE_CHANNEL)          /**< Bits per LED command */
#define LED_IF_RMT_BUFFER_ITEMS  (LED_IF_NUM_LED * LED_IF_BITS_PER_LED_CMD)     /**< Number of items in buffer */
#define LED_IF_T0H               CONFIG_LED_IF_T0H                              /**< 0 bit high time */
#define LED_IF_T1H               CONFIG_LED_IF_T1H                              /**< 1 bit high time */
#define LED_IF_TL                CONFIG_LED_IF_TL                               /**< low time (either bit) */

#define LED_IF_CONCURRENCY       CONFIG_LED_IF_CONCURRENCY                      /**< Concurrent mode */
#ifdef LED_IF_CONCURRENCY
#define LED_IF_THREAD_PRIORITY   CONFIG_LED_IF_THREAD_PRIORITY                  /**< Thread priority for scheduler */
#define LED_IF_THREAD_STACK_SIZE CONFIG_LED_IF_THREAD_STACK_SIZE                /**< Stack size for scheduler */
#endif // LED_IF_CONCURRENCY

#ifdef CONFIG_LED_IF_COLOR_CHANNEL_ORDER_RGB
#define LED_IF_COLOR_CHANNEL_ORDER_RGB
#endif // CONFIG_LED_IF_COLOR_CHANNEL_ORDER_RGB
#ifdef CONFIG_LED_IF_COLOR_CHANNEL_ORDER_GRB
#define LED_IF_COLOR_CHANNEL_ORDER_GRB
#endif // CONFIG_LED_IF_COLOR_CHANNEL_ORDER_GRB
#ifdef CONFIG_LED_IF_COLOR_CHANNEL_ORDER_RBG
#define LED_IF_COLOR_CHANNEL_ORDER_RBG
#endif // CONFIG_LED_IF_COLOR_CHANNEL_ORDER_RBG
#ifdef CONFIG_LED_IF_COLOR_CHANNEL_ORDER_BRG
#define LED_IF_COLOR_CHANNEL_ORDER_BRG
#endif // CONFIG_LED_IF_COLOR_CHANNEL_ORDER_BRG
#ifdef CONFIG_LED_IF_COLOR_CHANNEL_ORDER_BGR
#define LED_IF_COLOR_CHANNEL_ORDER_BGR
#endif // LED_IF_COLOR_CHANNEL_ORDER_BGR
#ifdef CONFIG_LED_IF_COLOR_CHANNEL_ORDER_GBR
#define LED_IF_COLOR_CHANNEL_ORDER_GBR
#endif // CONFIG_LED_IF_COLOR_CHANNEL_ORDER_GBR

////////////////////////////////////////////////////////////////////////////////
// Error handling
////////////////////////////////////////////////////////////////////////////////

/** @typedef The error type for any led related error */
typedef BaseType_t led_err_t;

#define LED_FAIL                                               -1               /**< Generic failure */
#define LED_OK                                              0x000               /**< Success */
#define LED_ALREADY_INITIALISED                             0x001               /**< Driver is already initialised */
#define LED_NOT_INITIALISED                                 0x002               /**< Driver is not initialised */
#define LED_GPIO_ERROR                                      0x010               /**< Generic GPIO error */
#define LED_GPIO_ERROR_CONFIG                               0x011               /**< GPIO config failed */
#define LED_GPIO_CONFIG_INVALID_ARG                         0x012               /**< GPIO config parameter error */
#define LED_GPIO_LEVEL_NUMBER_ERROR                         0x013               /**< LED GPIO level set error */
#define LED_RMT_DRIVER_NOT_INSTALLED                        0x014               /**< RMT driver not installed */
#define LED_RMT_TIMEOUT                                     0x015               /**< Exceeded 'wait_time' */
#define LED_RMT_INVALID_ARG                                 0x016               /**< RMT parameter error */
#define LED_RMT_INVALID_STATE                               0x017               /**< LED RMT driver already installed */
#define LED_RMT_NO_MEM                                      0x018               /**< Memory allocation failure */
#define LED_TIMEOUT                                         0x107               /**< LED write timed out */
#define LED_TASK_CREATION_FAILED                            0x110               /**< LED task could not be created */
#define LED_TASK_COMMAND_MAILBOX_CREATION_FAILED            0x111               /**< Command mailbox creation failed */
#define LED_TASK_SCHEDULER_STATE_MAILBOX_CREATION_FAILED    0x112               /**< S.state mailbox creation failed */
#define LED_PATTERN_ERROR                                   0x120               /**< Generic LED pattern error */
#define LED_PATTERN_DATA_QUEUE_FULL                         0x121               /**< Queue to store patterns is full */
#define LED_PATTERN_COMMAND_QUEUE_FULL                      0x122               /**< Queue to store commands is full */
#define LED_PATTERN_DATA_QUEUE_CREATION_FAILED              0x123               /**< Data queue creation failed */
#define LED_LEDS_BUSY                                       0x130               /**< Leds are busy */
#define LED_NO_SUCH_LED                                     0x131               /**< There is no led at this address */
#define LED_UNKOWN_LED_STATE                                0x132               /**< The state of the leds is unknown */
#define LED_STATE_MAILBOX_CREATION_FAILED                   0x133               /**< LED state mailbox creation failed*/

////////////////////////////////////////////////////////////////////////////////
// Debugging
////////////////////////////////////////////////////////////////////////////////
static const char TAG_LED_DRIVER[] = CONFIG_TAG_LED_IF;                         /**< TAG for ESP_LOGX macros. */


////////////////////////////////////////////////////////////////////////////////
// LED task & related things
////////////////////////////////////////////////////////////////////////////////
#if LED_IF_CONCURRENCY 
/**
 * @brief Scheduler to display patterns on leds
 * 
 * @todo:
 *  - pass queue handles as parameter to ledSchedulerTask to remove the 
 *      requirement for them to be globals.
 */
void ledSchedulerTask(void *pvParameters);

/**
 * @brief Handle for the pattern queue 
 * 
 * Led patterns to display are sent to the led scheduler task through a queue. 
 * The led scheduler will read patterns from the front of the queue and process
 * them.
 * 
 * @todo:
 *  - make private to driver
 */
extern QueueHandle_t qLEDPDataHandle;

/**
 * @brief Handle for the command queue
 * 
 */
extern QueueHandle_t qLEDPCommandHandle;
extern QueueHandle_t qLEDSchedulerStateMailbox;
#endif // LED_IF_CONCURRENCY 

/**
 * @brief Mailbox to keep the state of the leds
 * 
 * The static write_led function can run under
 * any task. Therefore updating the leds needs to ensure concurrent access. 
 * This is implemented as a mailbox (queue with a size of 1 where the writing
 * task only overvrites the value and the reading task only peeks the value)
 * 
 * @todo:
 *  - make the led state mailbox more private
 */
extern QueueHandle_t qLEDStatesMailbox;


////////////////////////////////////////////////////////////////////////////////
// LED states & patterns
////////////////////////////////////////////////////////////////////////////////

typedef struct rgb_led_pixel {
    uint32_t red : LED_IF_BITS_PER_COLOR;
    uint32_t green : LED_IF_BITS_PER_COLOR;
    uint32_t blue : LED_IF_BITS_PER_COLOR;
} rgbLEDPixel;

typedef struct hsv_led_pixel {
    uint32_t hue : LED_IF_BITS_PER_COLOR;
    uint32_t saturation : LED_IF_BITS_PER_COLOR;
    uint32_t value : LED_IF_BITS_PER_COLOR;
} hsvLEDPixel;

/**
 * @brief State of all the leds
 * 
 * This structure is used for indicating what the colors of each LED should be 
 * set to.
 */
typedef struct led_states {
    rgbLEDPixel led[LED_IF_NUM_LED] = {{.red = 0, .green = 0, .blue = 0 }};
} ledStates;


/** @typedef The function handle to update the leds 
 * 
 * This handle can be used in pattern functions as a placeholder for updating
 * the LEDs. 
 * 
 * NOTE: The handle will be filled in by the display driver, thus effectively
 *          hiding the direct hardware access to avoid race conditions and
 *          conflicts.
 */
typedef led_err_t (*hUpdateLED)(ledStates states);

/** @typedef The function type for the generator function of the led pattern
 * 
 * For a led pattern, a function that generates the pattern must be provided. 
 * The pointer to this function is passed as a member of the pattern function
 * and then used to execute the function. The only argument to the 
 * 
 * 
 * @param param A pointer to the argument (or argument list) provided to the 
 *              function. 
 */
typedef void (*pfLedPatternExecute)(hUpdateLED update_led, void *param);

/** @typedef The function type for the led pattern cleanup function 
 * 
 * If a pfLedPatternExecute function that generates the led pattern dynamically
 * allocates memory or takes ownership of dynamically allocated memory it must
 * free this memory. However, often, memory remains allocated to be reused in 
 * between runs of the function. In this case, a led pattern cleanup function 
 * must be provided to clean up after the pattern gets unloaded (either due to
 * reaching max repetitions or being interrupted).
 * 
 * @param param A pointer to the argument (or argument list) provided to the 
 *              cleanup function. 
 */
typedef void (*pfLEDPatternCleanup)(void *param);


/**
 * @typedef The type to describe a single led state within a discrete pattern
 *
 * Discrete patterns are patterns that are not described by a function but
 * rather by a sequence of states.
 * 
 * NOTE: The name "discrete pattern" is a bit arbitrary, as a function of course
 *       can implement a discrete pattern.
 */
typedef struct pattern_state {
    ledStates state;
    TickType_t xTicksDuration;
    struct pattern_state *next;
} ledPatternState;

// DANGER- Do not use infinite uninterruptable patterns.
/**
 * @typedef The to describe a led pattern
 * 
 * TODO - document more in depth
 * 
 * DANGER: Do not use infinite uninterruptable patterns, or the leds will be
 *          blocked indefinitly.
 * 
 */
typedef struct led_pattern {
    bool interruptable = true;                                                  // Only has effect in concurrent mode
    bool use_end_state = false;
    unsigned int repetitions = 0;                                               // 0 = infinite in concurrent mode, 0 in blocking
    pfLedPatternExecute pattern_function = NULL;
    void *pfArg = NULL;
    ledStates end_state;
    pfLEDPatternCleanup cleanup_function = NULL;
    
} ledPattern;

/**
 * @brief Class that allows generation of discrete led patterns
 * 
 * This class provides functionality to generate discrete led patterns. Discrete
 * led patterns are patterns that are defined by a list of states. Those states
 * are displayed each for a specified time and then the next state is displayed.
 *
 * The pattern generater will generate the specified list and the pattern 
 * function that is used to display the pattern. The generator will also create
 * a cleanup function, as the pattern is dinamically allocated.
 * 
 * Once the pattern and its properties such as repettitons and whether it is 
 * interruptable or not have been defined (or left to the defaults), the 
 * generate() function can be called to retrieve the pattern in the form of a
 * lep_pattern structure which can be directly fed to the led driver.
 * 
 * Generating a pattern does reset the pattern generator, such that it can be
 * safely used for the generation of a new pattern. If during the generation of
 * a pattern a new pattern shall be started or the current pattern generation
 * abortet for any reason, then reset() must be called to ensure all dynamically
 * allocated memory is properly freed.
 */
class LEDPatternGenerator {
    private:
        ledPattern pattern;
        ledPatternState *root_state = NULL;
        ledPatternState *tail_state = NULL;
        static void pf(hUpdateLED uf ,void *arg);
        static void cleanupPf(void *arg);
        void setDefaults();

    public:
        LEDPatternGenerator();          // Constructor
        ~LEDPatternGenerator();         // Deconstructor
        void reset();                   // Reset to start new pattern
        void addState(ledStates s, TickType_t d);     // Add a state to the pattern
        void addEndState(ledStates s);  // Add a end state to display if pattern completed
        void setRepetitions(int r);     // Set numbers of repetitions
        void setInterruptable(bool i);  // Set if the pattern is interruptable
        void generate(ledPattern *pgen);    // Get an instance of the pattern 
        ledPattern *generate();    // Get an instance of the pattern 

};


/**
 * @brief LED Driver
 * 
 * This class provides functionality to configure and deal with
 * the rgb leds. It is implemented as a singleton. 
 */
class ledDriver {
#if LED_IF_CONCURRENCY
    public:
        /**
         * @brief Enum describing the state of the scheduler.
         * 
         * This enum is used to describe the state of the led pattern scheduler.
         * The states are:
         *  - Running:  There is a pattern running on display
         *  - Stopped:  No pattern is scheduled for display, queue is empty
         *  - Paused:   No pattern is scheduled for display, but there are 
         *              patterns waiting to be scheduled.
         *  - Unknown:  The state is cannot be determined.
         */
        enum class ledPatternSchedulerState {
            starting,
            running,
            stopped,
            waiting,
            paused,
            unknown
        };

        /**
         * @brief Commands to influence the state of the led pattern scheduler.
         * 
         */
        enum class ledPatternCommand {
            start,
            stop,
            pause,
            resume,
            reset,
        };
#endif //LED_IF_CONCURRENCY

        /**
         * @brief Get the Instance object
         * 
         * Returns the handle to the instance of the driver object.
         * 
         * @return ledDriver& 
         */
        static ledDriver& getInstance() {
            static ledDriver _s_Instance;
            led_err_t status = LED_OK;

            if(!(_s_Instance._initialised)) {
                status = _s_Instance._init();
                if(status != LED_OK) {
                    ESP_LOGE(TAG_LED_DRIVER, "Could not reset LED Driver. (E:0x%x)", status);
                } else {
                    ESP_LOGV(TAG_LED_DRIVER, "LED driver reset.");
                }
            }
            return _s_Instance;
        }


        /** @brief Resets the LED hardware and driver
         *
         * This function allows to reset the LED hardware. It is not recommended 
         * to be used unless the driver ends up in a non recoverable failure 
         * state. 
         * Resetting the driver will disrupt any animation played on the leds by
         * any thread. It will reset the state of the hardware to the options 
         * chosen in menuconfig. Further it will flush all queues and - if 
         * running in concurrent mode - kill and restart the thread handling all
         * patterns.
         * 
         * Note: Reset is using a lock to ensure it is thread safe. 
         *
         * @return led_err_t 
         *              - LED_OK if successful
         *              - LED_RMT_INVALID_ARG if the parameter for the RMT 
         *                  device was errornoeous
         *              - LED_RMT_INVALID_STATE if the LED RMT driver already 
         *                  installed, call rmt_driver_uninstall first. 
         *              - LED_RMT_NO_MEM if not enough heap memory is available 
         *                  to instatiate RMT
         *              - LED_FAIL for any other failure
         *              - LED_TASK_CREATION_FAILED if concurrency mode is  
         *                  selected and task could not be created
         */
        led_err_t reset();

        /**
         * @brief Get the status of the driver
         * 
         * @return LED_OK if ok, LED_NOT_INITIALISED if the driver is not yet
         *          initialised.
         */
        led_err_t getStatus();

        /** @brief Set a new pattern
         *
         *  This function sets a new pattern to be displayed. If
         *  the running pattern is interruptable, the new pattern
         *  will start
         * 
         *  @param pattern the pattern to be set
         *  @return LED_OK if pattern was set, LED_PATTERN_QUEUE_FULL
         *          if pattern could not be set because of full pattern
         *          queue.
         */
        led_err_t patternSchedule(ledPattern pattern);

#if LED_IF_CONCURRENCY

        /**
         * @brief Start the led pattern scheduler
         * 
         * Sends a message to the scheduler to start the pattern. If the 
         * scheduler is stopped, the scheduler will start the next pattern if a 
         * pattern is queued. If there is no pattern queued, the scheduler will 
         * wait for one. If the scheduler is paused, the scheduler will resume 
         * the paused pattern. If no pattern is paused, it will wait for a new 
         * one. If the scheduler is starting, running, waiting or in unknown 
         * state, the command is ignored.
         * 
         * @return LED_OK on success
         * @return LED_PATTERN_COMMAND_QUEUE_FULL if the command queue is full 
         *          and the command canot be sent,
         */
        led_err_t patternStart();

        /**
         * @brief Stop the led pattern and the scheduler
         * 
         * Sends a message to the scheduler to stop the currently running 
         * pattern and go into stop state. Stopping the led pattern means it 
         * will be cleaned up and cannot be resumed.
         * 
         * @return LED_OK on success.
         * @return LED_PATTERN_COMMAND_QUEUE_FULL if the command queue is full 
         *          and the command canot be sent,
         */
        led_err_t patternStop();

        /**
         * @brief Pause the led pattern and the scheduler
         * 
         * Sends a message to the scheduler to pause the current pattern and 
         * put the scheduler in pased state. 
         * 
         * @return LED_OK on success.
         * @return LED_PATTERN_COMMAND_QUEUE_FULL if the command queue is full 
         *              and the command canot be sent.
         */
        led_err_t patternPause();

        /**
         * @brief Resume the led pattern and the scheduler
         * 
         * Sends a message to the scheduler to resumer the currently scheduler 
         * pattern. If there is no pattern scheduled, the scheduler will wait
         * for the next pattern to be scheduled and then process it.
         * 
         * @return LED_OK on success.
         * @return LED_PATTERN_COMMAND_QUEUE_FULL if the command queue is full 
         *          and the command canot be sent.
         */
        led_err_t patternResume();

        /**
         * @brief Clear all patterns in the queue
         * 
         * Clears all patterns from the scheduler queue.
         * 
         * @return LED_OK on success.
         * @return LED_PATTERN_COMMAND_QUEUE_FULL if the command queue is full 
         *          and the command canot be sent.
         */
        led_err_t patternClearAll();

        /**
         * @brief Get the state of the scheduler
         * 
         * Retrieves in what state the scheduler currently is.
         * 
         * @return ledPatternSchedulerState the state of the scheduler
         */
        ledPatternSchedulerState getPatternSchedulerState();
#endif //LED_IF_CONCURRENCY

        rgbLEDPixel convertToRGB(hsvLEDPixel hsv_pixel);

        /**
         * @brief Set the color of a single led
         * 
         * Setting a individual led to a single color will interrupt a running
         * pattern, if the pattern is interruptable. If the pattern is not 
         * interruptable, the led will be changed to the color once the active
         * pattern stops.
         * 
         * Setting an individual led does not affect the state of any of the
         * other leds.
         * 
         * @todo Most likely we should use a locking mechanism to read the scheduler
         *          state to ensure that the state does not change while we write a led.
         *          On the other hand, it might be fine as when the scheduler is in 
         *          stopped or paused state, it can not receive a command before the
         *          rmt signal is sent?
         * 
         * @param num   The number of the led to set (starts at 0),
         * @param red   The value for the red color component.
         * @param green The value for the green color component.
         * @param blue  The value for the blue color component.
         * 
         * @return      LED_OK if successful.
         * @return      LED_NO_SUCH_LED if invalid numerb of led.
         * @return      LED_LEDS_BUSY if the led is busy and canot be set.
         * @return      LED_RMT_INVALID_ARG if the parameter for the RMT device was 
         *                  errornoeous. 
         * @return      LED_RMT_TIMEOUT if the RMT device reached a timeout before state 
         *                  could be written. 
         * @return      LED_RMT_DRIVER_NOT_INSTALLED if the driver for the RMT device is 
         *                  not installed. LED_FAIL for any other failure.
         */
        led_err_t setLed(uint8_t num, rgbLEDPixel pixel);
        led_err_t setLed(uint8_t num, hsvLEDPixel pixel);

        /**
         * @brief Set colors for all leds
         * 
         * Setting all leds to the specified color.
         * 
         * @param states The pixel colors to be set for each led.
         * @return LED_OK if successful.
         * @return LED_RMT_INVALID_ARG if the parameter for the RMT device was errornoeous.
         * @return LED_RMT_TIMEOUT if the RMT device reached a timeout before state could be written.
         * @return LED_RMT_DRIVER_NOT_INSTALLED if the driver for the RMT device is not installed.
         * @return LED_FAIL for any other failure.
         * @return LED_LEDS_BUSY if the leds are busy and canot be set.
         */
        led_err_t setLeds(ledStates states);

        /**
         * @brief Get the state of the leds.
         * 
         * @param states The pixel colors of each led 
         * @return 
         */
        led_err_t getLeds(ledStates *states);

        /**
         * @brief Delted copy constructor
         * 
         * As this is a singleton, the copy constructor is deleted.
         */
        ledDriver(const ledDriver&) = delete;

        /**
         * @brief Deleted assignment constructor
         * 
         * @param  
         * @return 
         */
        ledDriver& operator=(const ledDriver&) = delete;

        /**
         * @brief Deleted constructor
         * 
         */
        ledDriver(ledDriver&&) = delete;

        /**
         * @brief Deleted assignment constructor
         * 
         * @return ledDriver& 
         */
        ledDriver& operator=(ledDriver&&) = delete;

        /**
         * @brief Destroy the led Driver::led Driver object
         * 
         * Frees the resources before object is destroyed. 
         * 
         * Note: We do not care about thread safety in this case, as the singleton is
         *          only destroyed at the end of the programm.
         */
        ~ledDriver();

    private: 

        /**
         * @brief Flag to show if driver is properly initialised
         * 
         */
        bool _initialised = false;  
        
        /**
         * @brief Mutex to lock resources
         * 
         * This mutex allows to lock resources to ensure the driver supports
         * concurrenc.
         * 
         * @todo:
         *  - Might want to use different mutexes for different resources to 
         *      make driver more efficient.
         * 
         */
        static std::mutex _mutx;                                                /**< Mutex for thread safety */

        /**
         * @brief Buffer for rmt channel.
         * 
         * This is the buffer which the hw peripheral will access while pulsing 
         * the output pin. Therefore, this buffer will also contain the last
         * state written to the LED (except before the first pattern is written).
         * 
         * Note: This does not mean, that the LEDs received and are displaying 
         *          teh according colours.
         */
        static rmt_item32_t led_data_buffer[LED_IF_RMT_BUFFER_ITEMS];
        

#if LED_IF_CONCURRENCY 
        TaskHandle_t xHandle = NULL;                                            /**< Handle for the LED thread */
#endif // LED_IF_CONCURRENCY

        /**
         * @brief Constructor made private as the driver is a singleton
         * 
         * The constructor initialises the hardware and creates the needed data 
         * structures and - if in concurrent mode - thread and queues.
         */
        ledDriver() {
            _init();
        }

        /**
         * @brief Initialises the led driver
         * 
         * This private function will initialise the led driver. If the driver
         * is already initialised, it will not do anything.
         * 
         * Note: As we the ledDriver is implemented as singleton and thus the 
         *          init function should not be calable by any other object than
         *          the single instance, we do not make it thread safe. However,
         *          this requires any calling public function to ensure init is
         *          only called in a thread safe maner.
         * 
         * @return led_err_t 
         *              - LED_OK if successful
         *              - LED_RMT_INVALID_ARG if the parameter for the RMT 
         *                  device was errornoeous
         *              - LED_RMT_INVALID_STATE if the LED RMT driver already 
         *                  installed, call rmt_driver_uninstall first. 
         *              - LED_RMT_NO_MEM if not enough heap memory is available 
         *                  to instatiate RMT
         *              - LED_FAIL for any other failure
         *              - LED_TASK_CREATION_FAILED if concurrency mode is  
         *                  selected and task could not be created
         */
        led_err_t _init();
        
        /**
         * @brief Deinitialises the driver
         * 
         * This function deinitialises the driver and frees all resources.
         * 
         * Note: This function is not thread save, so the caller must ensure thread
         *          safety by for example using a lock.
         * 
         * @return LED_OK if ok.
         */
        led_err_t _deinit();

        /**
         * @brief Write a state directly to the leds.
         * 
         * This function writes a single state to the leds. The state will remain on
         * display until another state is written (or the display of a pattern started).
         * 
         * @warning Do not use this function directly, only use it in patterns that are 
         *          then scheduled.
         * 
         * @todo Solve potential abuse of function conflicting with other writes.
         * 
         * @param state The state for the leds to display.
         * 
         * @return LED_OK if successful.
         * @return LED_RMT_INVALID_ARG if the parameter for the RMT device was errornoeous.
         * @return LED_RMT_TIMEOUT if the RMT device reached a timeout before state could be written.
         * @return LED_RMT_DRIVER_NOT_INSTALLED if the driver for the RMT device is not installed.
         * @return LED_FAIL for any other failure.
         */
        static led_err_t _write_leds(ledStates states);
};


