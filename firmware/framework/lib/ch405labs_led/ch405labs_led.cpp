/** 
 * @file        ch405_labs_led.cpp
 * @author      Florian Schütz (fschuetz@ieee.org)
 * @brief       Driver and pattern generations functions for driving addressable
 *              leds.
 * @version     0.9
 * @date        2022-08-07
 * @copyright   Copyright (c) 2022, Florian Schuetz, released under MIT license
 *
 * Implementation for the led driver. Details see ch405_labs_led.hpp
 * 
 * Notes:
 *  - Command queue can store one command. This is to avoid conflicting 
 *      commands from different threads.
 * 
 * @bug FIXED - There should be a memory leak where patterns get not properly 
 *          deallocated as when running for a long time, malloc fails.
 * 
 * @todo We could eliminate the LEDPatternGenerator if we directly made a 
 *          pattern object. This would be cleaner as we can ensure that the
 *          pfcleanup function gets called in the destructor.
 */

#include "ch405labs_led.hpp"


#if LED_IF_CONCURRENCY 

// Globals
QueueHandle_t qLEDPDataHandle = NULL;           // Handle to the data queue
QueueHandle_t qLEDPCommandHandle = NULL;        // Handle to the command queue
QueueHandle_t qLEDSchedulerStateMailbox = NULL; // Handle to scheduler mailbox
QueueHandle_t qLEDStatesMailbox = NULL;         // Handle to the states mailbox


/**
 * @brief Task to operate a scheduler to asynchronously display led patterns
 * 
 * This task receives patterns through a queue. Whenever the task succesfully
 * receives a pattern and the scheduler is in a running state, it will start 
 * executing it for the number of repetitions or, if it is defined 
 * interruptable, until it is interrupted in between two pattern runs.
 * 
 * If the scheduler is stopped or paused, the pattern remains in the queue. The
 * task will only check the queue (FIFO) when it is either already in running
 * state or it is started / resumed and thus transfers in running mode.
 * 
 * Interruptions of a currently displayed pattern can either happen by other 
 * patterns waiting in the queue or by pasing or stopping the scheduler. If 
 * another pattern is ready and the current pattern on display is interruptable 
 * and finished an execution run, the old pattern will be cleaned up and the new 
 * one will be scheduled. If a pattern is not interruptable, then a pattern is 
 * executed until it completes all its repetitions. CAUTION: Uninterruptable, 
 * infinite patterns will occupy the LEDs until stopped through a command.
 * 
 * LED patterns can be interrupted through pause or stop commands. A pause
 * command will stop the pattern after a completed run until the scheduler is
 * resumed. The led count will continue. A stop command will stop the currently
 * displayed pattern after a completed pattern run and unload it. When the
 * scheduler is put in run state again, the scheduler will check for a new 
 * pattern in the queue.
 * 
 * @param pvParameters Contains the function to write to the leds.
 * 
 * TODO:
 *      - Deal with undefined state
 */
void ledSchedulerTask(void *pvParameters) {

    hUpdateLED write_led = (hUpdateLED)pvParameters;

    // Initialise with dummy values. We need to make sure that the cleanup function
    // is null in order to not trip over the loop when no pattern is scheduled.
    ledPattern lp = {
        .interruptable = true,
        .use_end_state = false,
        .repetitions = 0,
        .pattern_function = NULL,
        .pfArg = NULL,
        .end_state { 0,0,0 },
        .cleanup_function = NULL,     
    };
    ledDriver::ledPatternCommand lc;
    BaseType_t xStatus;
    ledDriver::ledPatternSchedulerState schedulerState = 
        ledDriver::ledPatternSchedulerState::stopped;
    xQueueOverwrite( qLEDSchedulerStateMailbox, &schedulerState );
    int rep = 0;
    bool end_state_reached = true;


#ifdef CONFIG_DEBUG_STACK
    UBaseType_t uxHighWaterMark;

    /* Inspect our own high water mark on entering the task. */
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    ESP_LOGD(CONFIG_TAG_STACK, "ledSchedulerTask(): High watermark for stack at start "
        "is: %d", uxHighWaterMark);
#endif

    do {
        
        // Check if there is a new command. If we are in a non-running state, 
        // then we wait until a command is received. If we are in a running 
        // state, then we just quickly check if another command is waiting to be 
        // processed and continue if there is none.
        if(schedulerState == ledDriver::ledPatternSchedulerState::stopped ||
           schedulerState == ledDriver::ledPatternSchedulerState::paused) {
            xStatus = xQueueReceive( qLEDPCommandHandle, &lc, portMAX_DELAY);
        } else {
            xStatus = xQueueReceive( qLEDPCommandHandle, &lc, 0 );
        }

        // If we have a command message we process it.
        if(xStatus == pdPASS) {
            switch(lc) {
                // Start command reveived:
                //  - If the scheduler is in starting state and another start 
                //      command is received, continue startup
                //  - If scheduler state is stopped, we start the scheduler
                //  - If scheduler state is running, we ignore the command
                //  - If scheduler state is paused, we resume (assuming the user
                //      wanted to resume rather than start)
                case ledDriver::ledPatternCommand::start:
                    ESP_LOGD(TAG_LED_DRIVER, 
                                "Scheduler received start command.");
                    switch(schedulerState) {
                        case ledDriver::ledPatternSchedulerState::starting:
                            ESP_LOGD(TAG_LED_DRIVER, "Received multipe start "
                                "commands while starting. Continuing startup.");
                            [[fallthrough]];

                        case ledDriver::ledPatternSchedulerState::stopped:
                            ESP_LOGD(TAG_LED_DRIVER, 
                                        "Scheduler is stopped. Starting...");

                            // Wait for new pattern. (wait forever if needed.)
                            //
                            // Design decision here. We could introduce a waiting
                            // state that does not block on the pattern queue, 
                            // but this would lead us to a busy loop (bad
                            // for power consumption and responsiveness). Instead
                            // we wait for a pattern and then check if another 
                            // command was sent and deal with it. This will 
                            // block the command queue after one command has 
                            // been sent, but that should be ok from a user 
                            // perspective.
                            schedulerState = 
                                ledDriver::ledPatternSchedulerState::waiting;
                            xStatus = 
                                xQueueReceive( qLEDPDataHandle, &lp,  
                                                portMAX_DELAY );
                            if(xStatus == pdPASS) {
                                rep = 0;
                                end_state_reached = false;
                            } else {
                                ESP_LOGW(TAG_LED_DRIVER, "Tried to start "
                                    "scheduler, but received invalid signal "
                                    "from queue. Going back to stopped state.");
                                schedulerState = 
                                    ledDriver::ledPatternSchedulerState::stopped;
                            }

                            // As we waited potentially for a long time, we need
                            // to check if there is another command (potentially
                            // wanting to stop the scheduler or pause it.)
                            if(uxQueueMessagesWaiting(qLEDPCommandHandle) > 0) {
                                // Need to process the command first. We set the 
                                // scheduler state to starting and break to initiate
                                // next run.
                                schedulerState =
                                    ledDriver::ledPatternSchedulerState::starting;
                            } else {
                                // No command waiting, so we push the scheduler in
                                // running mode.
                                // All good, lets move on and run the scheduler
                                schedulerState = 
                                    ledDriver::ledPatternSchedulerState::running;
                            }
                            break;

                            
                        case ledDriver::ledPatternSchedulerState::running:
                            ESP_LOGD(TAG_LED_DRIVER, "Received start command "
                                "but scheduler already running. Ignoring...");
                            break;

                        case ledDriver::ledPatternSchedulerState::paused:
                            ESP_LOGD(TAG_LED_DRIVER, "Reveived start command. "
                                "Scheduler is paused. Resuming...");
                                schedulerState = 
                                    ledDriver::ledPatternSchedulerState::running;
                            break;

                        default:
                            ESP_LOGW(TAG_LED_DRIVER, "Received start command "
                                "but scheduler in unknown state. Ignoring...");
                            break;
                    }  
                    break;

                case ledDriver::ledPatternCommand::stop:
                    // Stopping unloads the currently scheduled pattern and 
                    // puts the scheduler in stop state.

                    // If the scheduler is in unknown state, then ignore this 
                    // command
                    if(schedulerState == 
                            ledDriver::ledPatternSchedulerState::unknown) {
                        break;
                    }

                    // Clean up the current pattern if a cleanup function is 
                    // provided
                    if( 
                        (schedulerState != ledDriver::ledPatternSchedulerState::stopped &&
                        schedulerState != ledDriver::ledPatternSchedulerState::waiting)
                        && lp.cleanup_function != NULL) {
#ifdef CONFIG_DEBUG_HEAP
                        ESP_LOGD(CONFIG_TAG_HEAP, "Pattern stopped, calling cleanup function at %p with arg at %p.\n", lp.cleanup_function, lp.pfArg);
#endif
                        (lp.cleanup_function)(lp.pfArg);
                    }
#ifdef CONFIG_DEBUG_HEAP
                    else {
                        ESP_LOGD(CONFIG_TAG_HEAP, "Pattern stopped, no need to clean up.\n");
                    }
#endif
                    // Just for safety, reset the count and end state to prepare for next function
                    // (Before scheduling the next pattern, we will also reset those parameters)
                    rep = 0;
                    end_state_reached = false;

                    
                    schedulerState = 
                        ledDriver::ledPatternSchedulerState::stopped;
                    break;

                case ledDriver::ledPatternCommand::pause:
                    // Go into paused state. No cleanup needed as wee keep the
                    // pattern ready.

                    // If the scheduler is in unknown state, then ignore this 
                    // command
                    if(schedulerState == 
                            ledDriver::ledPatternSchedulerState::unknown) {
                        break;
                    }

                    // Set to pause and as we break the loop also update
                    // scheduler state mailbox
                    schedulerState = 
                        ledDriver::ledPatternSchedulerState::paused;
                    xQueueOverwrite( qLEDSchedulerStateMailbox, &schedulerState );
                    continue;
                    break;

                case ledDriver::ledPatternCommand::resume:
                    // Simply switch the state to resume playing the pattern if
                    // we are in paused state. If we are not in paused state, we
                    // ignore the resume command.
                    if(schedulerState == 
                            ledDriver::ledPatternSchedulerState::paused) {
                        schedulerState = 
                        ledDriver::ledPatternSchedulerState::running;
                    }
                    break;

                case ledDriver::ledPatternCommand::reset:
                    // Reset command received. Reinitialising and going into 
                    // stopped state
                    // TODO
                    ESP_LOGE(TAG_LED_DRIVER, "Reset not yet implemented.");
                    [[fallthrough]]; 

                default:
                    ESP_LOGW(TAG_LED_DRIVER, "Received unknown command.");
                    break;
            }
        // TODO - move in preprocessor debug directive to not compile on release profile
        } else if(schedulerState == ledDriver::ledPatternSchedulerState::starting) {
            ESP_LOGE(TAG_LED_DRIVER, "Starting scheduler was interrupted, but "
                "no command in queue. This should not happen!!!");
        }

        // Update the schduler state mailbox
        xQueueOverwrite( qLEDSchedulerStateMailbox, &schedulerState );

        // If the scheduler is running, we work on displaying the pattern. If
        // not we simply do not execute the runtime and reenter the loop to 
        // listen for commands.
        if(schedulerState == ledDriver::ledPatternSchedulerState::running) {
            if(lp.interruptable || end_state_reached) {
                // This checks the queue for a new pattern if either the 
                // pattern ended or the pattern is interuptable between
                // runs. If the queue is empty and the pattern ended, then
                // this threads blocks until a new pattern is found. If the
                // queue is empty but the pattern has not yet finished (is
                // interruptable) then we immediately return and continue 
                // with the pattern.
                if(end_state_reached) {
                    // Clean up the current pattern if a cleanup function is 
                    // provided
                    if(lp.cleanup_function != NULL) {
    #ifdef CONFIG_DEBUG_HEAP
                        ESP_LOGD(CONFIG_TAG_HEAP, "End state reached, calling cleanup function at %p with arg at %p.\n", lp.cleanup_function, lp.pfArg);
    #endif
                        (lp.cleanup_function)(lp.pfArg);
                    }
    #ifdef CONFIG_DEBUG_HEAP
                    else {
                        ESP_LOGD(CONFIG_TAG_HEAP, "End state reached, no need to clean up.\n");
                    }
    #endif

                    // Wait for new pattern
                    xStatus = xQueueReceive( qLEDPDataHandle, &lp,  portMAX_DELAY );
                } else {
                    // Make a copy of the cleanup function and argument in case the
                    // pattern gets replaced
                    pfLEDPatternCleanup cleanup_function = lp.cleanup_function;
                    void *pfArg = lp.pfArg;

                    xStatus = xQueueReceive( qLEDPDataHandle, &lp, 0 );

                    // If xStatus is pdPass and there is a cleanup function, then we 
                    // need to clean up
                    if(xStatus == pdPASS && cleanup_function != NULL) {
    #ifdef CONFIG_DEBUG_HEAP
                        ESP_LOGD(CONFIG_TAG_HEAP, "Pattern interrupted. Cleaning up with function at %p with arg at %p.\n", cleanup_function, pfArg);
    #endif
                        (cleanup_function)(pfArg);
                    } 
    #ifdef CONFIG_DEBUG_HEAP
                    else if(xStatus == pdPASS) {
                        ESP_LOGD(CONFIG_TAG_HEAP, "Pattern interrupted. No need to cleanup.\n");
                    } else {
                        ESP_LOGD(CONFIG_TAG_HEAP, "Pattern not interrupted, going on.\n");
                    }
    #endif
                }
                if(xStatus == pdPASS) {
                    rep = 0;
                    end_state_reached = false;
                }
            }

            if(lp.repetitions == 0 || rep < lp.repetitions) {
                (lp.pattern_function)(write_led, lp.pfArg);
                rep++;
            } else {
                if(!end_state_reached) {
                    if(lp.use_end_state) {
                        write_led(lp.end_state);
                    } 
                    end_state_reached = true;
                }
            }
        }     

#ifdef CONFIG_DEBUG_STACK
    /* Inspect our own high water mark on entering the task. */
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    ESP_LOGD(CONFIG_TAG_STACK, "ledSchedulerTask(): High watermark for stack at end of loop is: %d", uxHighWaterMark);
#endif
    } while(true);
} 
#endif //LED_IF_CONCURRENCY 



/*
 * LED Pattern Generator
 */

/**
 * @brief Construct a new LEDPatternGenerator object
 * 
 */
LEDPatternGenerator::LEDPatternGenerator() {
    this->pattern.use_end_state = false;
    this->pattern.interruptable = true;
    this->pattern.repetitions = 0;
    this->pattern.pattern_function = this->pf;
}

/**
 * @brief Destroy the LEDPatternGenerator object
 * 
 * When destroying the LEDPatternGenerator object, we need to make sure that 
 * the pattern is properly cleaned up. For this we call the cleanup function.
 */
LEDPatternGenerator::~LEDPatternGenerator() {
    // In case we are in the midst of a pattern generation make sure to release
    // dynamically allocated memory.
    cleanupPf(this->root_state);
}

/**
 * @brief Reset all values to the default and free any allocated memory
 * 
 * This function resets the generator into its initial state to allow a new 
 * pattern to be gnenerated. Note that a reset also properly frees any 
 * dynamically allocated memory.
 * 
 */
void LEDPatternGenerator::reset() {
    // Reset will reset the generator into its initial state and 
    // free any allocated memory.
    cleanupPf(this->root_state);
    this->root_state = NULL;
    this->tail_state = NULL;
    this->setDefaults();
}                  

/**
 * @brief 
 * 
 * @param s 
 * @param d 
 */
void LEDPatternGenerator::addState(ledStates s, TickType_t d) {
    if(this->root_state == NULL) {
        root_state = new ledPatternState;
        this->tail_state = root_state;
    } else {
        this->tail_state->next = new ledPatternState;
        this->tail_state = this->tail_state->next;
    }
    this->tail_state->state = s;
    this->tail_state->xTicksDuration = d;
    (this->tail_state)->next = NULL;
}

/**
 * @brief 
 * 
 * @param s 
 */
void LEDPatternGenerator::addEndState(ledStates s) {
    this->pattern.end_state = s;
    this->pattern.use_end_state = true;
}

/**
 * @brief 
 * 
 * @param r 
 */
void LEDPatternGenerator::setRepetitions(int r) {
    this->pattern.repetitions = r;
}

/**
 * @brief 
 * 
 * @param i 
 */
void LEDPatternGenerator::setInterruptable(bool i) {
    this->pattern.interruptable = i;
}

/**
 * @brief 
 * 
 * @param pgen 
 * 
 * @todo Change such that it returns the pattern
 */
void LEDPatternGenerator::generate(ledPattern *pgen) {
    
    // Set the values in the pattern variable provided
    pgen->interruptable = this->pattern.interruptable;
    pgen->repetitions = this->pattern.repetitions;
    pgen->use_end_state = this->pattern.use_end_state;
    pgen->end_state = this->pattern.end_state;
    pgen->pattern_function = this->pf;
    pgen->pfArg = root_state;
    pgen->cleanup_function = this->cleanupPf;

    // Reset the generator, but do not delete the pattern
    // list, as this must remain available. The thread 
    // displaying will cleanup the pattern. If the user does
    // not supply the pattern to the thread, the user is 
    // responsible to free the memory.
    this->root_state = NULL;
    this->tail_state = NULL;
    this->setDefaults();
}

/**
 * @brief 
 * 
 * @param pgen 
 * 
 * @throw std::bad_alloc
 * 
 * @todo Change such that it returns the pattern
 */
ledPattern *LEDPatternGenerator::generate() {
    // TODO use placement new and adjust function that frees the pattern.
    ledPattern *pgen = new ledPattern;

    // Set the values in the pattern variable provided
    pgen->interruptable = this->pattern.interruptable;
    pgen->repetitions = this->pattern.repetitions;
    pgen->use_end_state = this->pattern.use_end_state;
    pgen->end_state = this->pattern.end_state;
    pgen->pattern_function = this->pf;
    pgen->pfArg = root_state;
    pgen->cleanup_function = this->cleanupPf;

    // Reset the generator, but do not delete the pattern
    // list, as this must remain available. The thread 
    // displaying will cleanup the pattern. If the user does
    // not supply the pattern to the thread, the user is 
    // responsible to free the memory.
    this->root_state = NULL;
    this->tail_state = NULL;
    this->setDefaults();

    return pgen;
}

/**
 * @brief 
 * 
 * @param arg 
 */
void LEDPatternGenerator::pf(hUpdateLED uf, void *arg) {
    ledPatternState *current_state = (ledPatternState*)arg;
    
    while(current_state != NULL) {
        if(uf(current_state->state) != LED_OK) {
            ESP_LOGE(TAG_LED_DRIVER, "LEDPatternGenerator::pf: Writing LEDs failed.");
        }
        vTaskDelay(current_state->xTicksDuration); // TODO make parameter of struct
        current_state = current_state->next;
    }
}


/**
 * @brief 
 * 
 * @param arg 
 * 
 * TODO:
 *  - might want to make pattern_state doubly linked, such that we save processing time
 */
void LEDPatternGenerator::cleanupPf(void *arg) {
#ifdef CONFIG_DEBUG_HEAP
    ESP_LOGD(CONFIG_TAG_HEAP, "Starting pattern clenaup.\n");
#endif
   
    ledPatternState *root_state = (ledPatternState*)arg;
    ledPatternState *current_state = root_state;
    ledPatternState *prev_state = NULL;

#ifdef CONFIG_DEBUG_STACK
    UBaseType_t uxHighWaterMark;

    /* Inspect our own high water mark on entering the task. */
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    ESP_LOGD(CONFIG_TAG_STACK, "LEDPatternGenerator::cleanupPf(): High watermark for stack at start is: %d", uxHighWaterMark);
#endif

    current_state = root_state;
#ifdef CONFIG_DEBUG_HEAP
    ESP_LOGD(CONFIG_TAG_HEAP, "Starting to traverse states at root %p.\n", root_state);
#endif
    while(true) {
        if(current_state == NULL) {
            //Queue is empty, nothing to delete - Done
#ifdef CONFIG_DEBUG_HEAP
            ESP_LOGD(CONFIG_TAG_HEAP, "Queue is empty. Nothing to delete.\n");
#endif
            break;
        } else if (current_state->next == NULL) {
#ifdef CONFIG_DEBUG_HEAP
            ESP_LOGD(CONFIG_TAG_HEAP, "Deleting current state %p.\n", current_state);
#endif
            // TODO change to free when using placement new
            delete current_state;
            current_state = NULL;
            if(prev_state != NULL) {
                prev_state->next = NULL;
            } else {
                // We delted the root - Done
#ifdef CONFIG_DEBUG_HEAP
                ESP_LOGD(CONFIG_TAG_HEAP, "Done deleting states.\n");
#endif
                break;
            }
            
            // Restart from beginning
            current_state = root_state;
            prev_state = NULL;
        } else {
#ifdef CONFIG_DEBUG_HEAP
            ESP_LOGD(CONFIG_TAG_HEAP, "Continuing search for last state in queue. Checking %p.\n", current_state->next);
#endif
            prev_state = current_state;
            current_state = current_state->next;
        }
    }

#ifdef CONFIG_DEBUG_HEAP
    ESP_LOGD(CONFIG_TAG_HEAP, "Succesfully cleaned up.\n");
#endif

#ifdef CONFIG_DEBUG_STACK
    /* Inspect our own high water mark on entering the task. */
    uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
    ESP_LOGD(CONFIG_TAG_STACK, "LEDPatternGenerator::cleanupPf(): High watermark for stack at end is: %d", uxHighWaterMark);
#endif
}


/**
 * @brief Set all variables to default values without freeing memory.
 * 
 */
void LEDPatternGenerator::setDefaults() {
    this->pattern.cleanup_function = this->cleanupPf;
    this->pattern.pattern_function = this->pf;
    this->pattern.use_end_state = false;
    this->pattern.repetitions = 0;
    this->pattern.pfArg = NULL;
    this->pattern.interruptable = true;
}


/****************************************************************
 * LED Driver
 ****************************************************************/

led_err_t ledDriver::getStatus() {
    if(this->_initialised) {
        return LED_OK;
    } else {
        return LED_NOT_INITIALISED;
    }
}

/**
 * @brief Destroy the led Driver::led Driver object
 * 
 * Frees the resources before object is destroyed. 
 * 
 * Note: We do not care about thread safety in this case, as the singleton is
 *          only destroyed at the end of the programm.
 */
ledDriver::~ledDriver() {
    _deinit();
}


/**
 * @brief Initialize hardware and driver
 * 
 * Configure the GPIO pins for the LEDs.
 * 
 * @return led_err_t 
 *              - LED_OK if successful
 *              - LED_RMT_INVALID_ARG if the parameter for the RMT device was 
 *                  errornoeous
 *              - LED_RMT_INVALID_STATE if the LED RMT driver already installed, 
 *                  call rmt_driver_uninstall first. 
 *              - LED_RMT_NO_MEM if not enough heap memory is available to 
 *                  instatiate RMT
 *              - LED_FAIL for any other failure
 *              - LED_TASK_CREATION_FAILED if concurrency mode is selected and 
 *                  task could not be created
 */
led_err_t ledDriver::reset() {

    // We need to lock to ensure exclusive access first
    std::lock_guard<std::mutex> state_guard(_mutx);

    // Deiniti the driver - is always ok
    _deinit();

    // Now initialise the driver anew
    return _init();
}


/**
 * @brief Schedule a pattern to be displayed
 * 
 * If concurrent mode is used, this function sends a pattern to be displayed to 
 * the queue for the LED display. The pattern will then be displayed if the 
 * scheduler is running and once all patterns in front of it are either 
 * completely displayed or allow interruption of their execution and replacement 
 * by the next candidate. 
 * 
 * If blocking mode is used, this function directly displays the pattern.
 * 
 * @param p The pattern to be scheduled
 * @return led_err_t 
 */
led_err_t ledDriver::patternSchedule(ledPattern p) {
#if LED_IF_CONCURRENCY 

    BaseType_t xLEDDataStatus = xQueueSendToBack(qLEDPDataHandle, &p, 0);
    if(xLEDDataStatus != pdPASS) {
        return LED_PATTERN_DATA_QUEUE_FULL;
    }
    return LED_OK;

#else //LED_IF_CONCURRENCY

    hUpdateLED write_led = ledDriver::write_leds;

    for(int i = 0; i < p.repetitions; i++) {
        (p.pattern_function)(write_led, p.pfArg);
    } 

    if(p.use_end_state) {
        write_led(p.end_state);
    } 

    if(p.cleanup_function != NULL) {
        (p.cleanup_function)(p.pfArg);
    }
    return LED_OK;
    
#endif //LED_IF_CONCURRENCY
}

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
led_err_t ledDriver::patternStart() {
    ledDriver::ledPatternCommand cmd = ledDriver::ledPatternCommand::start;

    // TODO - we should check the current state of the scheduler and only start
    // if it is stopped
    BaseType_t xLEDCommandStatus = xQueueSendToBack(qLEDPCommandHandle, &cmd, 0);
    if(xLEDCommandStatus != pdPASS) {
        return LED_PATTERN_COMMAND_QUEUE_FULL;
    }
    return LED_OK;
}

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
led_err_t ledDriver::patternStop() {
    ledDriver::ledPatternCommand cmd = ledDriver::ledPatternCommand::stop;

    // TODO - we should check the current state of the scheduler and only stop
    // if it is running or paused
    BaseType_t xLEDCommandStatus = xQueueSendToBack(qLEDPCommandHandle, &cmd, 0);
    if(xLEDCommandStatus != pdPASS) {
        return LED_PATTERN_COMMAND_QUEUE_FULL;
    }
    return LED_OK;
}

 /**
  * @brief Pause the led pattern and the scheduler
  * 
  * Sends a message to the scheduler to pause the current pattern and 
  * put the scheduler in pased state. 
  * 
  * @return LED_OK on success.
  * @return LED_PATTERN_COMMAND_QUEUE_FULL if the command queue is full and the 
  *             command canot be sent,
  */
led_err_t ledDriver::patternPause() {
    ledDriver::ledPatternCommand cmd = ledDriver::ledPatternCommand::pause;

    // TODO - we should check the current state of the scheduler and only pause
    // if it is running
    BaseType_t xLEDCommandStatus = xQueueSendToBack(qLEDPCommandHandle, &cmd, 0);
    if(xLEDCommandStatus != pdPASS) {
        return LED_PATTERN_COMMAND_QUEUE_FULL;
    }
    return LED_OK;
}

/**
 * @brief 
 * 
 * @return led_err_t 
 */
led_err_t ledDriver::patternResume() {
    ledDriver::ledPatternCommand cmd = ledDriver::ledPatternCommand::resume;

    // TODO - we should check the current state of the scheduler and only resume
    // if it is paused 
    BaseType_t xLEDCommandStatus = xQueueSendToBack(qLEDPCommandHandle, &cmd, 0);
    if(xLEDCommandStatus != pdPASS) {
        return LED_PATTERN_COMMAND_QUEUE_FULL;
    }
    return LED_OK;
}

led_err_t ledDriver::patternClearAll() {
    ledDriver::ledPatternCommand cmd = ledDriver::ledPatternCommand::stop;

    // First, lets stop the scheduler
    // TODO - we should check if the scheduler is already stopped
    BaseType_t xStatus = xQueueSendToBack(qLEDPCommandHandle, &cmd, 0);
    if(xStatus != pdPASS) {
        return LED_PATTERN_COMMAND_QUEUE_FULL;
    }

    // Clear pattern queue - Since FreeRTOS V7.2.0 always returns pdPASS.
    xQueueReset(qLEDPDataHandle);

    return LED_OK;
}

/**
 * @brief Get the state of the scheduler
 * 
 * Retrieves in what state the scheduler currently is.
 * 
 * @return ledPatternSchedulerState the state of the scheduler
 */
ledDriver::ledPatternSchedulerState ledDriver::getPatternSchedulerState() {
    ledDriver::ledPatternSchedulerState state;
    if(xQueuePeek(qLEDSchedulerStateMailbox, &state, 0) == pdTRUE) {
        return state;
    } else {
        return ledDriver::ledPatternSchedulerState::unknown;
    }
}
#endif //LED_IF_CONCURRENCY


rgbLEDPixel ledDriver::convertToRGB(hsvLEDPixel hsv_pixel) {
    rgbLEDPixel rgb_pixel;

    // Map hue to the range [0,360] degrees
    uint8_t ranged_hue = hsv_pixel.hue % 360;

    uint32_t rgb_max = hsv_pixel.value * 2.55f;
    uint32_t rgb_min = rgb_max * (100 - hsv_pixel.saturation) / 100.0f;

    uint32_t i = ranged_hue / 60;
    uint32_t diff = ranged_hue % 60;

    uint32_t rgb_adj = (rgb_max - rgb_min) * diff / 60;

    switch (i) {
        case 0:
            rgb_pixel.red = rgb_max;
            rgb_pixel.green = rgb_min + rgb_adj;
            rgb_pixel.blue = rgb_min;
            break;
        case 1:
            rgb_pixel.red = rgb_max - rgb_adj;
            rgb_pixel.green = rgb_max;
            rgb_pixel.blue = rgb_min;
            break;
        case 2:
            rgb_pixel.red = rgb_min;
            rgb_pixel.green = rgb_max;
            rgb_pixel.blue = rgb_min + rgb_adj;
            break;
        case 3:
            rgb_pixel.red = rgb_min;
            rgb_pixel.green = rgb_max - rgb_adj;
            rgb_pixel.blue = rgb_max;
            break;
        case 4:
            rgb_pixel.red = rgb_min + rgb_adj;
            rgb_pixel.green = rgb_min;
            rgb_pixel.blue = rgb_max;
            break;
        default:
            rgb_pixel.red = rgb_max;
            rgb_pixel.green = rgb_min;
            rgb_pixel.blue = rgb_max - rgb_adj;
            break;
    }

    return rgb_pixel;
}

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
 * @param pixel The rgb pixel value to set.
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
led_err_t ledDriver::setLed(uint8_t num, rgbLEDPixel pixel) {

    // We need to lock to ensure exclusive access first
    std::lock_guard<std::mutex> state_guard(_mutx);

#if LED_IF_CONCURRENCY 
    ledDriver::ledPatternSchedulerState lpsState = ledDriver::getPatternSchedulerState();
    if(lpsState == ledPatternSchedulerState::paused ||
        lpsState == ledPatternSchedulerState::stopped) {
#endif //LED_IF_CONCURRENCY
            if(num > LED_IF_NUM_LED) {
                return LED_NO_SUCH_LED;
            }
            ledStates states;

            // Read the state of the leds
            getLeds(&states);
            states.led[num] = pixel;
            return _write_leds(states);
#if LED_IF_CONCURRENCY 
    } else {
        return LED_LEDS_BUSY;
    }
#endif //LEDS_IF_CONCURRENCY
}

/**
 * @brief Set colors for all leds
 * 
 * Setting all leds to the specified color.
 * 
 * @param states The colors to be set for each led.
 * @return LED_OK if successful.
 * @return LED_RMT_INVALID_ARG if the parameter for the RMT device was errornoeous.
 * @return LED_RMT_TIMEOUT if the RMT device reached a timeout before state could be written.
 * @return LED_RMT_DRIVER_NOT_INSTALLED if the driver for the RMT device is not installed.
 * @return LED_FAIL for any other failure.
 * @return LED_LEDS_BUSY if the leds are busy and canot be set.
 */
led_err_t ledDriver::setLeds(ledStates states) {
#if LED_IF_CONCURRENCY 
    ledDriver::ledPatternSchedulerState lpsState = ledDriver::getPatternSchedulerState();
    if(lpsState == ledPatternSchedulerState::paused ||
        lpsState == ledPatternSchedulerState::stopped) {
#endif //LED_IF_CONCURRENCY
            return _write_leds(states);
#if LED_IF_CONCURRENCY 
    } else {
        return LED_LEDS_BUSY;
    }
#endif //LED_IF_CONCURRENCY
}

/**
 * @brief Get the current colors of all the leds
 * 
 * @param states Pointer to the variable where to store the states. 
 * @return LED_OK if state is read successully.
 * @return LED_UNKNOWN_LED_STATE if state could not be determined.
 */
led_err_t ledDriver::getLeds(ledStates *states) {
    // Peek the queue to see if there is a state message. (After first
    // use of the leds this will always be true as we are using a letterbox 
    // which is only updated if leds are set anew.)
    if(xQueuePeek( qLEDStatesMailbox, states, 0 ) == pdTRUE) {
        return LED_OK;
    } else {
        return LED_UNKOWN_LED_STATE;
    }
}

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
 * @param states The state for the leds to display.
 * 
 * @return LED_OK if successful.
 * @return LED_RMT_INVALID_ARG if the parameter for the RMT device was errornoeous.
 * @return LED_RMT_TIMEOUT if the RMT device reached a timeout before state could be written.
 * @return LED_RMT_DRIVER_NOT_INSTALLED if the driver for the RMT device is not installed.
 * @return LED_FAIL for any other failure.
 */
led_err_t ledDriver::_write_leds(ledStates states) {

    esp_err_t error;

    // Setup RMT data buffer
    for (uint32_t led_no = 0; led_no < LED_IF_NUM_LED; led_no++) {

        // Map the red channel to hardware capability
        uint32_t bits_to_send_red;
        uint32_t bits_to_send_green;
        uint32_t bits_to_send_blue;


        // We need to map the intervals. To avoid using floats we differentiate
        // the cases. Case one is where the internal representation and the 
        // hardware representation use the same amount of bits. Then its a 
        // simple copy. If the internal representation is bigger than the 
        // hardware representation we must intrapolate the internal value, if it
        // is smaller, we must extrapolite.
        // Note, that we use the fact that 2^x / 2^y = 2^(x-y) and 1 << n = 2^n
        // and that the integer division rounds down for this to work.
        if(LED_IF_BITS_PER_COLOR == LED_IF_BITS_RED_CHANNEL) {
            bits_to_send_red = states.led[led_no].red;
        } else if (LED_IF_BITS_PER_COLOR > LED_IF_BITS_RED_CHANNEL) {
            // Internal representation is bigger than hardware resolution.
            int32_t slope = 1 << (LED_IF_BITS_PER_COLOR - LED_IF_BITS_RED_CHANNEL);
            bits_to_send_red = states.led[led_no].red / slope;
        } else {
            // Hardware resolution is bigger than internal capability
            int32_t slope = 1 << (LED_IF_BITS_RED_CHANNEL - LED_IF_BITS_PER_COLOR);
            bits_to_send_red =states.led[led_no].red * slope;
        }

        if(LED_IF_BITS_PER_COLOR == LED_IF_BITS_GREEN_CHANNEL) {
            bits_to_send_green = states.led[led_no].green;
        } else if (LED_IF_BITS_PER_COLOR > LED_IF_BITS_GREEN_CHANNEL) {
            // Internal representation is bigger than hardware resolution
            int32_t slope = 1 << (LED_IF_BITS_PER_COLOR - LED_IF_BITS_GREEN_CHANNEL);
            bits_to_send_green = states.led[led_no].green / slope;
        } else {
            // Hardware resolution is bigger than internal capability
            int32_t slope = 1 << (LED_IF_BITS_GREEN_CHANNEL - LED_IF_BITS_PER_COLOR);
            bits_to_send_green =states.led[led_no].green * slope;
        }

        if(LED_IF_BITS_PER_COLOR == LED_IF_BITS_BLUE_CHANNEL) {
            bits_to_send_blue = states.led[led_no].blue;
        } else if (LED_IF_BITS_PER_COLOR > LED_IF_BITS_BLUE_CHANNEL) {
            // Internal representation is bigger than hardware resolution
            int32_t slope = 1 << (LED_IF_BITS_PER_COLOR - LED_IF_BITS_BLUE_CHANNEL);
            bits_to_send_blue = states.led[led_no].blue / slope;
        } else {
            // Hardware resolution is bigger than internal capability
            int32_t slope = 1 << (LED_IF_BITS_BLUE_CHANNEL - LED_IF_BITS_PER_COLOR);
            bits_to_send_blue =states.led[led_no].blue * slope;
        }

#ifdef LED_IF_COLOR_CHANNEL_ORDER_GRB
        uint32_t mask = 1 << (LED_IF_BITS_GREEN_CHANNEL - 1);
        for (uint32_t bit = 0; bit < LED_IF_BITS_GREEN_CHANNEL; bit++) {
            uint32_t bit_is_set = bits_to_send_green & mask;
            ledDriver::led_data_buffer[led_no * LED_IF_BITS_PER_LED_CMD + bit] = bit_is_set ?
                                                        (rmt_item32_t){{{LED_IF_T1H, 1, LED_IF_TL, 0}}} : 
                                                        (rmt_item32_t){{{LED_IF_T0H, 1, LED_IF_TL, 0}}};
            mask >>= 1;
        }

        mask = 1 << (LED_IF_BITS_RED_CHANNEL - 1);
        for (uint32_t bit = 0; bit < LED_IF_BITS_RED_CHANNEL; bit++) {
            uint32_t bit_is_set = bits_to_send_red & mask;
            ledDriver::led_data_buffer[led_no * LED_IF_BITS_PER_LED_CMD + LED_IF_BITS_GREEN_CHANNEL + bit] = bit_is_set ?
                                                        (rmt_item32_t){{{LED_IF_T1H, 1, LED_IF_TL, 0}}} : 
                                                        (rmt_item32_t){{{LED_IF_T0H, 1, LED_IF_TL, 0}}};
            mask >>= 1;
        }

        mask = 1 << (LED_IF_BITS_BLUE_CHANNEL - 1);
        for (uint32_t bit = 0; bit < LED_IF_BITS_BLUE_CHANNEL; bit++) {
            uint32_t bit_is_set = bits_to_send_blue & mask;
            ledDriver::led_data_buffer[led_no * LED_IF_BITS_PER_LED_CMD + LED_IF_BITS_GREEN_CHANNEL + LED_IF_BITS_RED_CHANNEL + bit] = bit_is_set ?
                                                        (rmt_item32_t){{{LED_IF_T1H, 1, LED_IF_TL, 0}}} : 
                                                        (rmt_item32_t){{{LED_IF_T0H, 1, LED_IF_TL, 0}}};
            mask >>= 1;
        }
#else
#error Color order of led not supported.
#endif // LED_IF_COLOR_CHANNEL_ORDER_...
    }

    // Start RMT transmission
    error = rmt_write_items(LED_IF_RMT_TX_CHANNEL, ledDriver::led_data_buffer, LED_IF_RMT_BUFFER_ITEMS, false);
    if(error != ESP_OK)  {
        return LED_RMT_INVALID_ARG;
    }
    error = rmt_wait_tx_done(LED_IF_RMT_TX_CHANNEL, portMAX_DELAY);
    switch(error) {
        case ESP_OK:
            // Update the led state to reflect current colors displayed
            xQueueOverwrite( qLEDStatesMailbox, &states );
            return LED_OK;
        
        case ESP_ERR_INVALID_ARG:
            return LED_RMT_INVALID_ARG;
        
        case ESP_ERR_TIMEOUT:
            return LED_RMT_TIMEOUT;
        
        case ESP_FAIL:
            return LED_RMT_DRIVER_NOT_INSTALLED;
        
        default:
            return LED_FAIL; 
    }
    return error;
}

/**
 * @brief Mutex to lock the driver
 * 
 * To ensure the driver can be used concurrently, this mutex will be used to
 * lock if needed.
 * 
 */
std::mutex ledDriver::_mutx{};

/**
 * @brief Buffer for rmt channel
 * 
 * This is the buffer which the hw peripheral will access while pulsing 
 * the output pin. Therefore, this buffer will also contain the last
 * state written to the LED (except before the first pattern is written).
 * 
 * Note: This does not mean, that the LEDs received and are displaying 
 *          the according colours.
 */
rmt_item32_t ledDriver::led_data_buffer[LED_IF_RMT_BUFFER_ITEMS];

/**
 * @brief Initialises the led driver
 * 
 * This private function will initialise the led driver. 
 * 
 * Note: As we the ledDriver is implemented as singleton and thus the 
 *          init function should not be calable by any other object than
 *          the single instance, we do not make it thread safe. However,
 *          this requires any calling public function to ensure init is
 *          only called in a thread safe maner.
 * 
 * @return led_err_t 
 *              - LED_OK if successful
 *              - LED_RMT_INVALID_ARG if the parameter for the RMT device was 
 *                  errornoeous
 *              - LED_RMT_INVALID_STATE if the LED RMT driver already installed, 
 *                  call rmt_driver_uninstall first. 
 *              - LED_RMT_NO_MEM if not enough heap memory is available to 
 *                  instatiate RMT
 *              - LED_FAIL for any other failure
 *              - LED_TASK_CREATION_FAILED if concurrency mode is selected and 
 *                  task could not be created
 */
led_err_t ledDriver::_init() {
    // Do not initialise twice!
    if(this->_initialised) {
        return LED_ALREADY_INITIALISED;
    }
   
    // Variable for error tracking
    esp_err_t error = ESP_OK;

     // Configure output pins (LED_CTRL_EN, LED_CTRL_DATA)
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = ((1ULL<<LED_IF_CONTROL_EN_PIN) 
                                | (1ULL<<LED_IF_RMT_TX_PIN));
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;

    error = gpio_config(&io_conf);
    if(error != ESP_OK) {
        return LED_GPIO_CONFIG_INVALID_ARG;
    }

    // Activate the LED control
    error = gpio_set_level(LED_IF_CONTROL_EN_PIN, 1);
    if(error != ESP_OK) {
        return LED_GPIO_LEVEL_NUMBER_ERROR;
    }
    vTaskDelay(5 / portTICK_PERIOD_MS);
    
    // Configure RMT for sending data to leds
    rmt_config_t rmt_conf = {                                                
        .rmt_mode = RMT_MODE_TX,                     
        .channel = LED_IF_RMT_TX_CHANNEL,                       
        .gpio_num = LED_IF_RMT_TX_PIN,                            
        .clk_div = 2,                             
        .mem_block_num = 1, //3                         
        .flags = 0,                                  
        .tx_config = {                               
            .carrier_freq_hz = 38000,                
            .carrier_level = RMT_CARRIER_LEVEL_HIGH, 
            .idle_level = RMT_IDLE_LEVEL_LOW,        
            .carrier_duty_percent = 33,
#if SOC_RMT_SUPPORT_TX_LOOP_COUNT
            .loop_count = 0,               
#endif              
            .carrier_en = false,                     
            .loop_en = false,                        
            .idle_output_en = true,                  
        }                                            
    };

    error = rmt_config(&rmt_conf);
    if(error != ESP_OK) {
        switch(error) {
            case ESP_ERR_INVALID_ARG:
                return LED_RMT_INVALID_ARG;
                break; 
            default:
                return LED_FAIL;
        }
    }
    error = rmt_driver_install(rmt_conf.channel, 0, 0);

    if(error != ESP_OK) {
        ESP_LOGE(TAG_LED_DRIVER, "RMT Initialization failed.");
        switch(error) {
            case ESP_ERR_INVALID_STATE:
                return LED_RMT_INVALID_STATE;
            case ESP_ERR_NO_MEM:
                return LED_RMT_NO_MEM;
            case ESP_ERR_INVALID_ARG:
                return LED_RMT_INVALID_ARG;
            default:
                return LED_FAIL;
        }
    }

    ESP_LOGD(TAG_LED_DRIVER, "RMT Initialisation successful.");

    // Create the mailbox for the state of the leds
    qLEDStatesMailbox = xQueueCreate( 1, sizeof(ledStates) );
    if(qLEDStatesMailbox == NULL) {
        rmt_driver_uninstall(rmt_conf.channel);
        return LED_STATE_MAILBOX_CREATION_FAILED;
    }

#if LED_IF_CONCURRENCY 
    // Generate data and control message queues and start the thread

    // First try to create the queues for message passing
    qLEDPDataHandle = xQueueCreate( 5,  sizeof(ledPattern) );
    if(qLEDPDataHandle == NULL) {
        vQueueDelete(qLEDStatesMailbox);
        qLEDStatesMailbox = NULL;
        rmt_driver_uninstall(rmt_conf.channel);
        return LED_PATTERN_DATA_QUEUE_CREATION_FAILED;
    }
    qLEDPCommandHandle = xQueueCreate( 1, sizeof(ledPatternCommand));
    if(qLEDPCommandHandle == NULL) {
        vQueueDelete(qLEDPDataHandle);
        qLEDPDataHandle = NULL;
        vQueueDelete(qLEDStatesMailbox);
        qLEDStatesMailbox = NULL;
        rmt_driver_uninstall(rmt_conf.channel);
        return LED_TASK_COMMAND_MAILBOX_CREATION_FAILED;
    }

    qLEDSchedulerStateMailbox = xQueueCreate( 1, sizeof(ledDriver::ledPatternSchedulerState));
    if(qLEDSchedulerStateMailbox == NULL) {
        vQueueDelete(qLEDPCommandHandle);
        qLEDPCommandHandle = NULL;
        vQueueDelete(qLEDPDataHandle);
        qLEDPDataHandle = NULL;
        vQueueDelete(qLEDStatesMailbox);
        qLEDStatesMailbox = NULL;
        rmt_driver_uninstall(rmt_conf.channel);
        return LED_TASK_SCHEDULER_STATE_MAILBOX_CREATION_FAILED;
    }
    // Scheduler is not yet started, so we set it to stopped
    ledDriver::ledPatternSchedulerState schedulerState = 
        ledDriver::ledPatternSchedulerState::stopped;
    xQueueOverwrite( qLEDSchedulerStateMailbox, &schedulerState );

    hUpdateLED uf = ledDriver::_write_leds;

    BaseType_t task_error = xTaskCreate( 
        ledSchedulerTask, 
        "LEDScheduler", 
        LED_IF_THREAD_STACK_SIZE + configSTACK_OVERHEAD_TOTAL, 
        (void *) uf, 
        LED_IF_THREAD_PRIORITY, 
        &xHandle);
    if(task_error != pdPASS) {
        vQueueDelete(qLEDSchedulerStateMailbox);
        qLEDSchedulerStateMailbox = NULL;
        vQueueDelete(qLEDPCommandHandle);
        qLEDPCommandHandle = NULL;
        vQueueDelete(qLEDPDataHandle);
        qLEDPDataHandle = NULL;
        vQueueDelete(qLEDStatesMailbox);
        qLEDStatesMailbox = NULL;
        rmt_driver_uninstall(rmt_conf.channel);
        return LED_TASK_CREATION_FAILED;
    } 
    
    this->_initialised = true;
    return LED_OK;

#else //LED_IF_CONCURRENCY

    ESP_LOGW(LED_IF_TAG, "ledDriver running in blocking mode!");
    if(error == ESP_OK) {
        this->_initialised = true;
    }
    return error;

#endif //LED_IF_CONCURRENCY
}

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
led_err_t ledDriver::_deinit() {

#if LED_IF_CONCURRENCY 
    // Delete the scheduler and the queues
    if(xHandle != NULL) {
        vTaskDelete(xHandle);
    }

    if(qLEDPCommandHandle != NULL) {
        vQueueDelete(qLEDPCommandHandle);
        qLEDPCommandHandle = NULL;
    }

    if(qLEDPDataHandle != NULL) {
        vQueueDelete(qLEDPDataHandle);
        qLEDPDataHandle = NULL;
    }

    if(qLEDSchedulerStateMailbox != NULL) {
        vQueueDelete(qLEDSchedulerStateMailbox);
        qLEDSchedulerStateMailbox = NULL;
    }

#endif // LED_IF_CONCURRENCY 

    if(qLEDStatesMailbox != NULL) {
        vQueueDelete(qLEDStatesMailbox);
        qLEDStatesMailbox = NULL;
    }

    // Uninstall rmt driver
    rmt_driver_uninstall(LED_IF_RMT_TX_CHANNEL);

    // No need to change the configuration of the GPIOs

    // Set the state of the driver to not initialised
    _initialised = false;

    return LED_OK;
}
