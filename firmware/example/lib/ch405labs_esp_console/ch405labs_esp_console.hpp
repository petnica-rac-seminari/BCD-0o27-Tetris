/** 
 * @file ch405labs_esp_console.hpp
 * @author Florian Schütz (fschuetz@ieee.org)
 * @brief Functions to manage a serial console on esp32.
 * @version 0.1
 * @date 2023-05-01
 * @copyright Modifications copyright (c) 2023 Florian Schütz, released under MIT license
 * @copyright Orignal code copyright (c) 2016-2021 Espressif Systems (Shanghai) CO LTD, released under Apache-2.0 license
 *
 * This class implements functionality to extend the espidf console module. It 
 * provides the following functionality:
 *  - Command execution: Is pretty self explanatory. However, command execution
 *      can also be disabled. This comes in handy if there are console listeners
 *      active.
 *  - Input forwarding: Input entered on the command can be forwarded to console
 *      listeners. A console listener provides a queue handle and the module 
 *      will forward the command to this queue. Multiple console listeners can 
 *      be registered. However, a single console listener can also request 
 *      exclusive access. In that case, all other console listneres will not 
 *      receive input until the exclusive console listener is deregistered.
 *
 *  Original code by Espressif Systems:
 *      SPDX-FileCopyrightText: 2016-2021 Espressif Systems (Shanghai) CO LTD
 *      SPDX-License-Identifier: Apache-2.0
 * 
 * Modifications from original code:
 *  - Names of functions to avoid clash with orginial function names when 
 *      including esp-idf components.
 *  - Slight modifications to allow for asynchronous linennoise function (see
 *      ch405lab_linenoise) to be used.
 *  - Added possibility to forward console input to subscribers (from concurrent
 *      threads).
 */
#pragma once

#include "freertos/FreeRTOS.h"
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_log.h>
#include <errno.h>
#include <cstring>
#include <unistd.h>
#include <esp_vfs_dev.h>
#include "driver/uart.h"
#include "ch405_labs_esp_console.h"
#include "ch405labs_linenoise.h"


////////////////////////////////////////////////////////////////////////////////
// Menuconfig options
////////////////////////////////////////////////////////////////////////////////
#define COMMAND_PROMPT_STR                  CONFIG_COMMAND_PROMPT_STR
#define CONSOLE_MAX_COMMAND_LINE_LENGTH     CONFIG_CONSOLE_MAX_COMMAND_LINE_LENGTH
#define COMMAND_TASK_STACK_SIZE             CONFIG_COMMAND_TASK_STACK_SIZE
#define COMMAND_HISTORY_PATH                CONFIG_COMMAND_HISTORY_PATH
#define COMMAND_MAX_LISTENERS               100                                 // TODO make configurable

#if CONFIG_CONSOLE_LOG_LEVEL == 0
#define CONSOLE_LOG_LEVEL esp_log_level_t::ESP_LOG_NONE
#elif CONFIG_CONSOLE_LOG_LEVEL == 1
#define CONSOLE_LOG_LEVEL esp_log_level_t::ESP_LOG_ERROR
#elif CONFIG_CONSOLE_LOG_LEVEL == 2
#define CONSOLE_LOG_LEVEL esp_log_level_t::ESP_LOG_WARN
#elif CONFIG_CONSOLE_LOG_LEVEL == 3
#define CONSOLE_LOG_LEVEL esp_log_level_t::ESP_LOG_INFO
#elif CONFIG_CONSOLE_LOG_LEVEL == 4
#define CONSOLE_LOG_LEVEL esp_log_level_t::ESP_LOG_DEBUG
#elif CONFIG_CONSOLE_LOG_LEVEL == 5
#define CONSOLE_LOG_LEVEL esp_log_level_t::ESP_LOG_VERBOSE
#endif //CONFIG_CONSOLE_LOG_LEVEL

////////////////////////////////////////////////////////////////////////////////
// Error handling
////////////////////////////////////////////////////////////////////////////////

/** @typedef The error type for any wifi related error */
typedef BaseType_t console_err_t;

#define CONSOLE_FAIL                           -1                               /**< Generic failure */
#define CONSOLE_OK                          0x000                               /**< Success */
#define CONSOLE_NOT_INITIALIZED             0x001                               /**< Console not properly initialised */
#define CONSOLE_ERR_NO_MEM                  0x101                               /**< Out of memory */
#define CONSOLE_ERR_INVALID_ARG             0x102                               /**< Invalid argument */
#define CONSOLE_ERR_INVALID_STATE           0x103                               /**< Invalid state */           

////////////////////////////////////////////////////////////////////////////////
// Debugging
////////////////////////////////////////////////////////////////////////////////
static const char TAG_CONSOLE[] = CONFIG_TAG_CONSOLE;                           /**< TAG for ESP_LOGX macro. */

////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////
#define INVALID_HANDLE                      (COMMAND_MAX_LISTENERS + 1)         /**< An invalid handle */
#define TEMP_HANDLE                         COMMAND_MAX_LISTENERS               /**< A teporary handle */

namespace espconsole {

    typedef uint32_t consoleForwarderHandle_t;                                  /** Handle for forwarders */

    /**
     * @brief Paramters for forwarding commands to console listeners
     * 
     */
    typedef struct forwarder_param {
        consoleForwarderHandle_t *phForwarder;                                  /** The handle of the console listener */                                           
        QueueHandle_t hData;                                                    /** The queue handle to forward input to */
        QueueHandle_t hSignal;                                                  /** The queue handle to */
    } forwarderParam_t;

    /**
     * @brief Signals types to influence the behaviour of the console.
     * 
     * Console signals are used to influence the behaviour of the console.
     * The signals work as follows:
     * 
     * stop:                Stops the console.
     * stop_cmd_exec:       Stops the inbuilt command execution of the console.
     * resume_cmd_exec:     Resumes inbuilt command exectuion of the console.
     * redirect:            Registers a new input redirect client.
     * stop_redirect:       Deregisterst an existing input redirect client.
     * exclusive_redirect:  Register an exclusive input redirect client. Only
     *                      one exclusive input redirect client can ba active at
     *                      any given time.
     * stop_exclusive_redirect: Deregisters the exclusive input redirect client.
     *                      This reactivates normal redirects.
     */
    enum class consoleSignalType {
        stop,                                                                   /** Stop the console */
        stop_cmd_exec,                                                          /** Stop inbuilt command execution */
        resume_cmd_exec,                                                        /** Resume inbuilt command execution */
        redirect,                                                               /** Register input redirect client */
        stop_redirect,                                                          /** Deregister input redirect client */
        exclusive_redirect,                                                     /** Register exclusive input redirect */
        stop_exclusive_redirect,                                                /** Deregister exclusive redirect */
    };


    /**
     * @brief Return values for signals influencing the behaviour of the console
     * 
     * TODO
     */
    enum class consoleSignalRetvalType {
        not_defined,
        ok,
        fail,
        already_locked_exclusively,
        invalid_parameter,
        not_a_registered_consumer,
        no_more_memory,
        invalid_signal
    };
    
    /**
     * @brief Command to influence console behaviour
     * 
     */
    typedef struct console_signal {
        espconsole::consoleSignalType type;
        forwarderParam_t param;
    } consoleSignal;

    /**
     * @brief Return value for a signal to show success / failure
     * 
     */
    typedef struct console_signal_retval {
        espconsole::consoleSignalRetvalType type;
        consoleForwarderHandle_t recipient;
        // TODO maybe add unique id bound to signal
    } consoleSignalRetval;

    /**
     * @brief 
     * 
     * NOTE: Make sure to free the data.
     * 
     * TODO: maybe add free function
     */
    typedef struct console_data {
        char raw_content[CONSOLE_MAX_COMMAND_LINE_LENGTH];
    } consoleData;

    typedef struct redirect_consumer {
        QueueHandle_t qConsumer;
        consoleForwarderHandle_t hForwarder;
        struct redirect_consumer *prev;
        struct redirect_consumer *next;
    } redirectConsumer;

    /**
     * @brief Handle for the console listener signal queue
     * 
     * This handle is used to send signals to the console listeners. Signals
     * can be used to change the behaviour of the console listener. 
     */
    extern QueueHandle_t qConsoleSignalHandle;

    /**
     * @brief Handle for the console listener return value signal queue 
     * 
     * TODO
     */
    extern QueueHandle_t qConsoleSignalRetvalHandle;

    /**
     * @brief The task to handle user input on the console
     * 
     * This task listens for command on the UART console. If a command is 
     * entered the task will handle it depending on the configuration. 
     * 
     * If command execution is active, the user input will be tried to be 
     * executed as a command. For this the user input is split into an argc / 
     * argv argument list and the command is looked up in the list of registered
     * components. If the command is not found or could not execute successful, 
     * a failure message will print the reason to the console. If successful, 
     * the command is run.
     * 
     * If any forwarders are registered, then the entered input is forwarded to
     * those. In the special case where there is an exclusive forwarder, the
     * input is only forwarded to this forwarder and the others are ignored.
     * 
     * Note:    Running a command blocks the consoleListener task until the 
     *          command terminates and returns. During this time, no further 
     *          input or signal can be processed.
     * 
     * Note II: If forwarding fails, because the queue of the forwarder to 
     *          receive input is full the input is silently dropped.
     * 
     * @param args The prompt to be displayed for user input (must be a zero 
     *              terminated char array).
     */
    void consoleListener(void *args);


    /**
     * @brief Controller class for the console
     * 
     * Implemented as a singleton.
     */
    class consoleController {
        public:

            /**
             * @brief Get the Instance object
             * 
             * Returns the handle to the instance of the controller object.
             * 
             * @return ledDriver& 
             */
            static consoleController& getInstance() {
                static consoleController _s_Instance;
                return _s_Instance;
            }

            /**
             * @brief Get the status of the consoleController
             * 
             * @return CONSOLE_OK if ok
             * @return CONSOLE_NOT_INITIALIZED if the controler is not yet 
             *  initialised.
             */
            console_err_t getStatus();


            console_err_t registerCommand(const char *command, 
                ch405_labs_esp_console_cmd_func_t func, const char *help = NULL, const char *hint = NULL,
                void *argtable = NULL);
            

            /**
             * @brief Delted copy constructor
             * 
             * As this is a singleton, the copy constructor is deleted.
             */
            consoleController(const consoleController&) = delete;

            /**
             * @brief Deleted assignment constructor
             * 
             * @param  
             * @return 
             */
            consoleController& operator=(const consoleController&) = delete;

            /**
             * @brief Deleted constructor
             * 
             */
            consoleController(consoleController&&) = delete;

            /**
             * @brief Deleted assignment constructor
             * 
             * @return ledDriver& 
             */
            consoleController& operator=(consoleController&&) = delete;

            console_err_t start();
            console_err_t stop();           // Always blocking?

            console_err_t requestInputRedirect(QueueHandle_t qData, QueueHandle_t qSignal, consoleForwarderHandle_t *hForwarder, bool exclusive  = false);
            console_err_t requestInputRedirectAsync(QueueHandle_t qData, QueueHandle_t qSignal, consoleForwarderHandle_t *hForwarder, bool exclusive = false);
            console_err_t getInputRedirectStatusAsync(consoleForwarderHandle_t *hForwarder);
            console_err_t cancelInputRedirect(consoleForwarderHandle_t *hForwarder);

            console_err_t stopCommandExecution(consoleForwarderHandle_t *hForwarder);
            console_err_t enableCommandExecution(consoleForwarderHandle_t *hForwarder);


        private:
            /**
             * @brief Constructor made private as the driver is a singleton
             * 
             * The constructor initialises the hardware and creates the needed data 
             * structures and - if in concurrent mode - thread and queues.
             */
            consoleController() {
            }

            console_err_t _waitRetval(consoleForwarderHandle_t *hForwarder);

            console_err_t _deinit();

            /**
             * @brief Flag to show if controller is properly initialised
             * 
             */
            bool _initialised = false;  

            TaskHandle_t _cmdTaskHandle = NULL;
    };

}