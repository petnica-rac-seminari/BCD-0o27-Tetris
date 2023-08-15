/** 
 * @file ch405labs_esp_console.cpp
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
 * 
 * The original code is released under the Apache-2.0 license, the modificaitons
 * are released under the MIT license.
 */

#include "ch405labs_esp_console.hpp"

namespace espconsole {

    QueueHandle_t qConsoleSignalHandle = NULL;                                  // Handle to the signal queue
    QueueHandle_t qConsoleSignalRetvalHandle = NULL;                            // Handle to the signal retval queue

    /**
     * @brief Internal helper function to translate esp_err_t into console_err_t
     * 
     * @param e The esp error.
     * @return The matching console error.
     */
    inline console_err_t _translate_esp_err_t(esp_err_t e) {
        switch (e) {
            case ESP_OK:
                return CONSOLE_OK;
                break;
            case ESP_ERR_NO_MEM:
                return CONSOLE_ERR_NO_MEM;
                break;
            case ESP_ERR_INVALID_ARG:
                return CONSOLE_ERR_INVALID_ARG;
                break;
            case ESP_ERR_INVALID_STATE:
                return CONSOLE_ERR_INVALID_STATE;
                break;
            default:
                return CONSOLE_FAIL;
        }
    }

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
    void consoleListener(void *args) {                                         

#ifdef CONFIG_DEBUG_STACK
        /* Inspect our own high water mark on entering the task. */
        UBaseType_t uxHighWaterMark;

        uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
        ESP_LOGD(CONFIG_TAG_STACK, 
            "consoleListener(): High watermark for stack at start is: %d",
                uxHighWaterMark);
#endif //CONFIG_DEBUG_STACK
  
        BaseType_t xStatus;
        consoleSignal csig;
        consoleSignalRetval cret = {
            .type = consoleSignalRetvalType::not_defined,
            .recipient = INVALID_HANDLE
        };
        redirectConsumer *redirect_consumers = NULL;
        redirectConsumer *exclusive_consumer = NULL;
        bool exec_commands = true;
        char *prompt = (char *)args; 

        struct ch405_labs_linenoiseState ls;
        char buf[1024];
        char *line = NULL;
        ch405_labs_linenoiseEditStart(&ls,-1,-1,buf,sizeof(buf),prompt);

        while(true) {
            // Read a line (if available) 
            line = ch405_labs_linenoiseEditFeed(&ls);

            // Check if we received a new instruction on how to handle input.
            xStatus = xQueueReceive( qConsoleSignalHandle, &csig, 0);

            if(xStatus == pdPASS) {
                // Set the handle of the sender of the signal to its handle to 
                // allow identifying whom for the return value is.
                // This currently assumes, any signaler can only send one signal
                // and then must wait for a reply before sending another one. 
                // This is guaranteed due to the implementation of the letterbox 
                // scheme.
                cret.recipient = *(csig.param.phForwarder);
                switch(csig.type) {
                    // Start command reveived:
                    //  - If the scheduler is in starting state and another start 
                    //      command is received, continue startup
                    //  - If scheduler state is stopped, we start the scheduler
                    //  - If scheduler state is running, we ignore the command
                    //  - If scheduler state is paused, we resume (assuming the user
                    //      wanted to resume rather than start)
                    case consoleSignalType::stop:
                    {
                        ESP_LOGD(TAG_CONSOLE, "Terminating console"); 
                        cret.type = (ch405_labs_esp_console_deinit() == ESP_OK ? consoleSignalRetvalType::ok : consoleSignalRetvalType::fail);
                        break;
                    }
                    case consoleSignalType::stop_cmd_exec:
                    {
                        ESP_LOGD(TAG_CONSOLE, "Stopping execution of console commands. (only redirect)");
                        exec_commands = false;
                        cret.type = consoleSignalRetvalType::ok;
                        // TODO notify all listeners
                        break;
                    }
                    case consoleSignalType::resume_cmd_exec:
                    {
                        ESP_LOGD(TAG_CONSOLE, "Resuming execution of console commands");
                        exec_commands = true;
                        cret.type = consoleSignalRetvalType::ok;
                        break;
                    }
                    case consoleSignalType::exclusive_redirect:
                    {
                        // First, assign a temporary handle
                        //
                        // This will allow the caller to react to a failure
                        // should any occure.
                        // TODO assign proper temp handles (must be random from
                        // a pool)
                        cret.recipient = TEMP_HANDLE;

                        if(csig.param.phForwarder == NULL) {
                            ESP_LOGE(TAG_CONSOLE, "Invalid parameter, ignoring...");
                            cret.type = consoleSignalRetvalType::invalid_parameter;
                            // TODO verify - we cannot set a handle, hat to do?
                        } else if(exclusive_consumer) {
                            ESP_LOGE(TAG_CONSOLE, "Already locked exclusively.");
                            cret.type = consoleSignalRetvalType::already_locked_exclusively;
                            *(csig.param.phForwarder) = TEMP_HANDLE;
                        } else {
                            redirectConsumer *rc = (redirectConsumer *)malloc(sizeof(redirectConsumer));
                            if(rc != NULL) {
                                rc->hForwarder = 0;
                                rc->qConsumer = (QueueHandle_t)csig.param.hData;
                                exclusive_consumer = rc;
                                *(csig.param.phForwarder) = 0;
                                cret.recipient = 0;
                                cret.type = consoleSignalRetvalType::ok;
                            } else {
                                ESP_LOGE(TAG_CONSOLE, "Could not register exclusive consumer. Out of memory...");
                                cret.type = consoleSignalRetvalType::no_more_memory;
                                *(csig.param.phForwarder) = TEMP_HANDLE;
                            }
                        }
                        break;
                    }
                    case consoleSignalType::redirect:
                    {
                        if(csig.param.phForwarder == NULL) {
                            ESP_LOGE(TAG_CONSOLE, "Invalid parameter, ignoring...");
                            cret.type = consoleSignalRetvalType::invalid_parameter;
                            // TODO - we cannot assign a handle, is this a problem?
                        } else {
                            redirectConsumer *rc = (redirectConsumer *)malloc(sizeof(redirectConsumer));
                            if(rc != NULL) {
                                rc->next = NULL;
                                rc->prev = NULL;
                                rc->qConsumer = (QueueHandle_t)csig.param.hData;

                                if(redirect_consumers == NULL) {
                                    rc->hForwarder = 1;
                                    *(csig.param.phForwarder) = 1;
                                    redirect_consumers = rc;

                                    cret.recipient = 1;
                                } else {
                                    redirectConsumer *trc = redirect_consumers;
                                    int i = 1;
                                    while(trc->next != NULL) {
                                        i++;
                                        trc = trc->next;
                                    }
                                    *(csig.param.phForwarder) = i;
                                    rc->hForwarder = i;
                                    trc->next = rc;
                                    rc->prev = trc;

                                    cret.recipient = i;
                                }

                                cret.type = consoleSignalRetvalType::ok;
                            } else {
                                ESP_LOGE(TAG_CONSOLE, "Could not register consumer. Out of memory...");
                                cret.type = consoleSignalRetvalType::no_more_memory;
                                *(csig.param.phForwarder) = TEMP_HANDLE;
                            }
                        }
                        break;
                    }
                    case consoleSignalType::stop_redirect:
                    {
                        if(csig.param.phForwarder == NULL) {
                            ESP_LOGE(TAG_CONSOLE, "Invalid parameter, ignoring...");
                            cret.type = consoleSignalRetvalType::invalid_parameter;
                        } else {
                            bool found = false;
                            if(*(csig.param.phForwarder) == 0) {
                                // Exclusive forwarder has special handle 0
                                if(exclusive_consumer->hForwarder == *(csig.param.phForwarder)) {
                                    free(exclusive_consumer);
                                    exclusive_consumer = NULL;
                                    found = true;

                                    // TODO signal end of exclusivity to all listeners
                                    cret.type = consoleSignalRetvalType::ok;
                                } else {
                                    // should never happen
                                    ESP_LOGE(TAG_CONSOLE, "Exclusive forwarder does not have handle 0");
                                    cret.type = consoleSignalRetvalType::invalid_parameter;
                                }
                            } else {
                                redirectConsumer *trc = redirect_consumers;
                                while(trc != NULL) {
                                    ESP_LOGI(TAG_CONSOLE, "Current forwarder is %lu / Looking for %lu\n", trc->hForwarder, *(csig.param.phForwarder));
                                    if(trc->hForwarder == *(csig.param.phForwarder)) {
                                        found = true;
                                        if(trc->prev == NULL) {
                                            // Its the first element
                                            redirect_consumers = trc->next;
                                        } else {
                                            // Not the first element
                                            trc->prev = trc->next;
                                        }
                                        free(trc);
                                        cret.type = consoleSignalRetvalType::ok;
                                        break;
                                    }

                                    trc = trc->next;
                                }
                            }
                           
                            if(!found) {
                                ESP_LOGE(TAG_CONSOLE, "Could not cancel input redirect. Consumer not in list.");
                                cret.type = consoleSignalRetvalType::not_a_registered_consumer;
                            }         
                        }
                        break;
                    }
                    default:
                    {
                        ESP_LOGW(TAG_CONSOLE, "Reveived unknown signal. Ignoring...");
                        cret.type = consoleSignalRetvalType::invalid_signal;
                    }
                }

                // Send the return value for the signal
                //
                // TODO find a good strategy to ensure the reply can be sent. 
                // Because if the reply canot be sent, the caller may block
                // forever if issueing a blocking call. Currently we just block
                // until the queue is empty.
                BaseType_t xConsoleSignalRetvalStatus = 
                    xQueueSendToBack(qConsoleSignalRetvalHandle, &cret, 0);
                if(xConsoleSignalRetvalStatus != pdPASS) {
                    // TODO Proper retry strategy
                    ESP_LOGE(TAG_CONSOLE, "Failed to send signal return value. Retrying indefinitly...");
                    do {
                        xQueueSendToBack(qConsoleSignalRetvalHandle, &cret, pdMS_TO_TICKS(100));
                    } while(xConsoleSignalRetvalStatus != pdPASS);
                } 
            } 

            // Nothing to be read from stdin, so lets just continue
            if(line == NULL) {
                vTaskDelay(pdMS_TO_TICKS(50));
                continue;
            }


            // ASYNC TEST START part 3
            if(line != ch405_labs_linenoiseEditMore) {
                ch405_labs_linenoiseEditStop(&ls);
            // ASYNC TEST END part 3

            // Add the command to the history, whether it is valid or not
            if (strlen(line) > 0) {
                ch405_labs_linenoiseHistoryAdd(line);
    #if CONFIG_COMMAND_STORE_HISTORY
                // Save history to filesystem
                ch405_labs_linenoiseHistorySave(COMMAND_HISTORY_PATH); // TODO
    #endif
            }

            if(exclusive_consumer) {
                ESP_LOGD(TAG_CONSOLE, "Forwarding to exclusive consumer.");
                consoleData cd;
                strcpy(cd.raw_content, line);
                if(xQueueSend(exclusive_consumer->qConsumer, &cd, 0) != pdPASS) {
                    ESP_LOGE(TAG_CONSOLE, "Could not forward input. Queue is full.");
                }
            } else if(redirect_consumers != NULL) {
                ESP_LOGD(TAG_CONSOLE, "Forwarding to listeninc consumers.");
                redirectConsumer *rc = redirect_consumers;
                consoleData cd;
                while(rc != NULL) {
                    strcpy(cd.raw_content, line);
                    if(xQueueSend(rc->qConsumer, &cd, 0) != pdPASS) {
                        ESP_LOGE(TAG_CONSOLE, "Could not forward input. Queue is full.");
                    }
                    rc = rc->next;
                }
            }

            if(exec_commands) {
                // try to run the command
                int ret;
                esp_err_t err = ch405_labs_esp_console_run(line, &ret);
                if(err == ESP_ERR_NOT_FOUND) {
                    printf("Yeah, right. You are a great hacker. Learn the commands first!\n");
                } else if (err == ESP_ERR_INVALID_ARG) {
                    // command was empty
                    printf("Seriously? Empty command? I mean, nice try. But no!\n");
                    fflush(stdout);
                } else if (err == ESP_OK && ret != ESP_OK) {
                    printf("Command returned non-zero error code: 0x%x (%s)\n", ret, esp_err_to_name(ret));
                }
            }

            // linenois allocate the line to the buffer. Free it
            ch405_labs_linenoiseFree(line);

            // ASYNC TEST START part 4
            ch405_labs_linenoiseEditStart(&ls,-1,-1,buf,sizeof(buf), prompt);
            }
            // ASYNC TEST END part 4

    #ifdef CONFIG_DEBUG_STACK
        /* Inspect our own high water mark on iteration. */
        uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
        ESP_LOGD(CONFIG_TAG_STACK, "consoleListener(): High watermark for stack after iteration is: %d", uxHighWaterMark);
    #endif
        }

        ESP_LOGE(TAG_CONSOLE, "Error or end-of-input, terminating console"); 
        ch405_labs_esp_console_deinit();
    }

    console_err_t consoleController::getStatus() {
        if(!_initialised) {
            return CONSOLE_NOT_INITIALIZED;
        }
        return CONSOLE_OK;
    }


    console_err_t consoleController::registerCommand(const char *command, 
            ch405_labs_esp_console_cmd_func_t func, const char *help, 
            const char *hint, void *argtable) {

        const ch405_labs_esp_console_cmd_t cmd = {
            .command = command,
            .help = help,
            .hint = hint,
            .func = func,
            .argtable = argtable
        };

        return _translate_esp_err_t(ch405_labs_esp_console_cmd_register(&cmd));
    }

    /**
     * @brief Start the console
     * 
     * @return console_err_t 
     */
    console_err_t consoleController::start() {

        // Note: Currently initialised means running, as there is no way to 
        //          initialise the UART at object creation without potentially
        //          crashing the ESP (if the object is created before the
        //          bootloader does its uart sheninanigas - eg. core dump init)
        if(_initialised) {
            ESP_LOGI(TAG_CONSOLE, "Console already running.");
            return CONSOLE_OK;
        }

        // Set the log level
        esp_log_level_set(TAG_CONSOLE, CONSOLE_LOG_LEVEL);

        // First drain stdout
        // We do not check for errors, as we do not really care at this point.
        fflush(stdout);
        fsync(fileno(stdout));

        // Disable buffering on stdin
        // We warn if we can not disable buffering. However, this is very unlikely
        // and thus we ignore the error.
        if(setvbuf(stdin, NULL, _IONBF, 0) != 0) {
            ESP_LOGW(TAG_CONSOLE, "Could not disable buffering on stdin. Continuing anyways.");
            printf("\n\nNot working\n\n");
        }

        // Set line endings to CR for stdout and CRLF for stdin -- TODO - maybe make configurable
        if(esp_vfs_dev_uart_port_set_rx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CR) != 0) {
            ESP_LOGE(TAG_CONSOLE, "Setting rx line endings failed: %s (%d)", esp_err_to_name(errno), errno);
            return CONSOLE_FAIL;
        }
        if(esp_vfs_dev_uart_port_set_tx_line_endings(CONFIG_ESP_CONSOLE_UART_NUM, ESP_LINE_ENDINGS_CRLF) != 0) {
            ESP_LOGE(TAG_CONSOLE, "Setting tx line endings failed: %s (%d)", esp_err_to_name(errno), errno);
        }

        // Configure UART. REF_TICK is used so that baud rate remains correct while APB frequency
        // is changing in light sleep mode
        const uart_config_t uart_config = {
                .baud_rate = CONFIG_ESP_CONSOLE_UART_BAUDRATE,
                .data_bits = UART_DATA_8_BITS,
                .parity = UART_PARITY_DISABLE,
                .stop_bits = UART_STOP_BITS_1,
                .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
                .rx_flow_ctrl_thresh = 0, 
#if SOC_UART_SUPPORT_REF_TICK
                .source_clk = UART_SCLK_REF_TICK,
#elif SOC_UART_SUPPORT_XTAL_CLK
                .source_clk = UART_SCLK_XTAL,
#endif
        };

        // Install UART driver for interrupt-driven reads and writes
        esp_err_t console_error = uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM, 256, 0, 0, NULL, 0); 
        if(console_error == ESP_OK) {
            console_error = uart_param_config(CONFIG_ESP_CONSOLE_UART_NUM, &uart_config);
            if(console_error != ESP_OK) {
                ESP_LOGE(TAG_CONSOLE, "Could not set uart parameters: %s (%d)", esp_err_to_name(console_error), console_error);
                uart_driver_delete(CONFIG_ESP_CONSOLE_UART_NUM);
                return CONSOLE_FAIL;
            }
        } else {
            ESP_LOGE(TAG_CONSOLE, "Failed to install uart driver: %s (%d)", esp_err_to_name(console_error), console_error);
            return CONSOLE_FAIL;
        }
        
        // Tell VFS to use UART driver
        esp_vfs_dev_uart_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);

        // Initialise the console
        ch405_labs_esp_console_config_t console_config = {
                .max_cmdline_length = 256,
                .max_cmdline_args = 9,
#if CONFIG_LOG_COLORS
                .hint_color = atoi(LOG_COLOR_CYAN),
#endif //CONFIG_LOG_COLORS
                .hint_bold = 0,
        };
        console_error = ch405_labs_esp_console_init(&console_config);

        if(console_error != ESP_OK) {
            ESP_LOGE(TAG_CONSOLE, "Failed to initialise console.");
            uart_driver_delete(CONFIG_ESP_CONSOLE_UART_NUM);
            return _translate_esp_err_t(console_error);
        }

            
        // Configure ch405_labs_linenoise line completion library 
        ch405_labs_linenoiseSetMultiLine(1);                   // Enable multiline editin

        // Tell ch405_labs_linenoise where to get command completions and hints 
        ch405_labs_linenoiseSetCompletionCallback(&ch405_labs_esp_console_get_completion);
        ch405_labs_linenoiseSetHintsCallback((ch405_labs_linenoiseHintsCallback*) &ch405_labs_esp_console_get_hint);

        // Set command history size
        ch405_labs_linenoiseHistorySetMaxLen(100);

        //send command maximum length
        ch405_labs_linenoiseSetMaxLineLen(console_config.max_cmdline_length);

        // Don't return empty lines
        //TODO reimplement - linenoiseAllowEmpty(false);

        // Register help command
        // TODO - check if we should really always do this
        ch405_labs_esp_console_register_help_command();

        // Set a prompt to be printed before each line
        const char* prompt = LOG_COLOR("95") COMMAND_PROMPT_STR LOG_RESET_COLOR;

        printf("\n"
            "Welcome the the BalcCon Cyberdeck 0o26.\n"
            "Type 'help' to get the list of commands.\n"
            "Use UP/DOWN arrows to navigate through command history.\n"
            "Press TAB when typing command name to auto-complete.\n"
            "\n"
            "NOTE: If your terminal is auto configuring and could not catch the"
            "required sequences then use \"restart\" command.\n");
        
        // Check if terminal supports escape sequences
        int probe_status = ch405_labs_linenoiseProbe();
        if (probe_status) {
            printf("\n"
                "Your terminal application does not support escape sequences.\n"
                "Line editing and history features are disabled.\n"
                "On Windows, try using Putty instead.\n");
            ch405_labs_linenoiseSetDumbMode(1);

#if CONFIG_LOG_COLORS
            /* Since the terminal doesn't support escape sequences,
            * don't use color codes in the prompt.
            */
            prompt = COMMAND_PROMPT_STR;
#endif //CONFIG_LOG_COLORSiseSetDumbMode(1);
        }


        // Start the signal queue and the signal retval queue
        qConsoleSignalHandle = xQueueCreate( 1, sizeof(consoleSignal));
        if(qConsoleSignalHandle == NULL) {
            ESP_LOGE(TAG_CONSOLE, "Failed to initialise console (signal queue).");
            uart_driver_delete(CONFIG_ESP_CONSOLE_UART_NUM);
            return CONSOLE_FAIL;
        }
        qConsoleSignalRetvalHandle = xQueueCreate( 1, sizeof(consoleSignalRetval));
        if(qConsoleSignalRetvalHandle == NULL) {
            ESP_LOGE(TAG_CONSOLE, "Failed to initialise console (signal retval queue).");
            vQueueDelete(qConsoleSignalHandle);
            uart_driver_delete(CONFIG_ESP_CONSOLE_UART_NUM);
            return CONSOLE_FAIL;
        }

        // Start the task handling the command line
        BaseType_t task_error = xTaskCreate( 
            espconsole::consoleListener, 
            "consoleListener", 
            COMMAND_TASK_STACK_SIZE + configSTACK_OVERHEAD_TOTAL, 
            (void *) prompt, 
            2, 
            &_cmdTaskHandle);
        if(task_error != pdPASS) {
            ESP_LOGE(TAG_CONSOLE, "Could not start terminal task.");
            return CONSOLE_FAIL;
        }

        _initialised = true;
        return CONSOLE_OK;
    }

    /**
     * @brief Stop the console
     * 
     * @bug Killing the console task may lead to memory leak as linenoisFree(line)
     *          may not be called. - FIXME
     * 
     * @return console_err_t 
     */
    console_err_t consoleController::stop() {

        if(!_initialised) {
            return CONSOLE_OK;
        }

        // First, kill the task
        // TODO - we might want to send a signal to allow for graceful shutdwon
        // BUG : reading stdin does not work after this. ch405_labs_linenoise might still be busy waiting? Investigate.
        //vTaskDelete(_cmdTaskHandle);
        consoleSignal cmd = {
            .type = consoleSignalType::stop,
            .param = {
                .phForwarder = 0,
                .hData = 0,
                .hSignal = 0,
            }
        };
        BaseType_t xConsoleCommandStatus = xQueueSendToBack(qConsoleSignalHandle, &cmd, 0);
        if(xConsoleCommandStatus != pdPASS) {
            ESP_LOGE(TAG_CONSOLE, "Could not send command.");
            return CONSOLE_FAIL; // TODO proper
        }

        // Deinitalise the console
        esp_err_t status = ch405_labs_esp_console_deinit();
        if(status != ESP_OK) {
            ESP_LOGE(TAG_CONSOLE, "Invalid status of esp console. Cannot deinit: %s (%d)", esp_err_to_name(status), status);
            return _translate_esp_err_t(status);
        }

        _initialised = false;   // TODO - make more granular

        // Delete the uart driver
        // This is a problem, as this will unlink stdin. Remove uart configuration from lib
        /*
        status = uart_driver_delete(CONFIG_ESP_CONSOLE_UART_NUM);
        if(status != ESP_OK) {
            ESP_LOGE(TAG_CONSOLE, "Invalid parameter. Cannot delete uart driver: %s (%d)", esp_err_to_name(status), status);
            return _translate_esp_err_t(status);
        }
        */

        return _translate_esp_err_t(status);
    }

    console_err_t consoleController::requestInputRedirect(QueueHandle_t qData, QueueHandle_t qSignal, consoleForwarderHandle_t *hForwarder, bool exclusive) {

        // We do not yet have a handle. The consoleListener thread will set the
        // handle value and we use this fact to determine if the reply is meant
        // for us. For this we initialise the handle initially to an impossible
        // value.
        
        *hForwarder = INVALID_HANDLE;

        consoleSignal cmd = {
            .type = consoleSignalType::redirect,
            .param = {
                .phForwarder = hForwarder,
                .hData = qData,
                .hSignal = qSignal,
            }
        };

        // Check if we need to do an exclusive redirect
        if(exclusive) {
            cmd.type = consoleSignalType::exclusive_redirect;
        }

        BaseType_t xConsoleCommandStatus = xQueueSendToBack(qConsoleSignalHandle, &cmd, 0);
        if(xConsoleCommandStatus != pdPASS) {
            ESP_LOGE(TAG_CONSOLE, "Could not send input redirect command.");
            return CONSOLE_FAIL; // TODO proper
        }

        return _waitRetval(hForwarder);
    }

    console_err_t consoleController::cancelInputRedirect(consoleForwarderHandle_t *hForwarder) {
        consoleSignal cmd = {
            .type = consoleSignalType::stop_redirect,
            .param = {
                .phForwarder = hForwarder,
                .hData = NULL,
                .hSignal = NULL
            }
        };

        ESP_LOGI(TAG_CONSOLE, "Canceling forwarder id %lu\n", *(cmd.param.phForwarder));
        // TODO maybe send blocking
        BaseType_t xConsoleCommandStatus = xQueueSendToBack(qConsoleSignalHandle, &cmd, 0);
        if(xConsoleCommandStatus != pdPASS) {
            ESP_LOGE(TAG_CONSOLE, "Could not send cancelInputRedirect command.");
            return CONSOLE_FAIL; // TODO proper
        }

        return _waitRetval(hForwarder);
    }
    
    console_err_t consoleController::stopCommandExecution(consoleForwarderHandle_t *hForwarder) {

        consoleSignal cmd = {
            .type = consoleSignalType::stop_cmd_exec,
            .param = {
                .phForwarder = hForwarder,
                .hData = NULL,
                .hSignal = NULL
            }
        };

        BaseType_t xConsoleCommandStatus = xQueueSendToBack(qConsoleSignalHandle, &cmd, 0);
        if(xConsoleCommandStatus != pdPASS) {
            ESP_LOGE(TAG_CONSOLE, "Could not send stopCommandExectuion command.");
            return CONSOLE_FAIL;
        }

        return _waitRetval(hForwarder);
    }

    console_err_t consoleController::enableCommandExecution(consoleForwarderHandle_t *hForwarder) {
        consoleSignal cmd = {
            .type = consoleSignalType::resume_cmd_exec,
            .param = {
                .phForwarder = hForwarder,
                .hData = NULL,
                .hSignal = NULL
            }
        };

        BaseType_t xConsoleCommandStatus = xQueueSendToBack(qConsoleSignalHandle, &cmd, 0);
        if(xConsoleCommandStatus != pdPASS) {
            ESP_LOGE(TAG_CONSOLE, "Could not send resumeCommandExectuion command.");
            return CONSOLE_FAIL;
        }

        return _waitRetval(hForwarder);
    }

    console_err_t consoleController::_waitRetval(consoleForwarderHandle_t *hForwarder) {
        // Wait for the return value
        //
        // As this is a blocking call, we wait forever for our return value. As
        // it may be that we peek a value that is not for us, we will sleep and
        // then peek again after a short timeout to see if a new value is 
        // available.
        consoleSignalRetval cret = {
            .type = consoleSignalRetvalType::not_defined,
            .recipient = INVALID_HANDLE
        };
        
        BaseType_t xConsoleSignalRetvalStatus = pdFAIL;
        xConsoleSignalRetvalStatus = 
            xQueuePeek(qConsoleSignalRetvalHandle, &cret, portMAX_DELAY);
        while(xConsoleSignalRetvalStatus != pdPASS 
            && cret.type == consoleSignalRetvalType::not_defined
            && cret.recipient != INVALID_HANDLE 
            && cret.recipient != *hForwarder) {

            vTaskDelay(pdMS_TO_TICKS(200));                                     // Give others time to remove their retval from the queue
            xConsoleSignalRetvalStatus = xQueuePeek(qConsoleSignalRetvalHandle, &cret, portMAX_DELAY);
        } 

        // If we end up here, this was our retval. Remove it from the queue
        if(xQueueReceive(qConsoleSignalRetvalHandle, &cret, portMAX_DELAY) != pdPASS 
            || cret.recipient != *hForwarder) {
            ESP_LOGE(TAG_CONSOLE, "This should never happen. Peeked a value but there is not a valid one?");
        }

        switch(cret.type) {
            case consoleSignalRetvalType::ok:
                return CONSOLE_OK;
                break;
            default:
                return CONSOLE_FAIL;
        }
    }

    console_err_t consoleController::_deinit() {
        return CONSOLE_FAIL;
    }

}