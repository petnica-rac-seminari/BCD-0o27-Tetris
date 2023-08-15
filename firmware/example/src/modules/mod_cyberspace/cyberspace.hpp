
#pragma once
#ifndef BCD_MODULE_SSH_CONNECT_H
#define BCD_MODULE_SSH_CONNECT_H

#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "st7735_bcd.hpp"
#include "gfx.hpp"
//#include "gfx_cpp14.hpp"
#include "fonts/Bm437_Acer_VGA_8x8.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "driver/uart.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_system.h"
#include "bootloader_random.h"

#include "esp_event.h"
#include "netdb.h" // gethostbyname

#include "libssh2_config.h"
#include <libssh2.h>
#include <libssh2_sftp.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>

#include "ch405labs_linenoise.h"

#include "ch405labs_led.hpp"
#include "ch405labs_esp_wifi.hpp"

#include "ch405labs_esp_console.hpp"


////////////////////////////////////////////////////////////////////////////////
// Namespaces
////////////////////////////////////////////////////////////////////////////////
using namespace espidf;
using namespace gfx;


////////////////////////////////////////////////////////////////////////////////
// Menuconfig options
////////////////////////////////////////////////////////////////////////////////
#define CYBERSPACE_SSH_HOST             CONFIG_CYBERSPACE_SSH_HOST
#define CYBERSPACE_SSH_PORT             CONFIG_CYBERSPACE_SSH_PORT
#define CYBERSPACE_SSH_USER             CONFIG_CYBERSPACE_SSH_USER
#ifdef CONFIG_CYBERSPACE_AUTH_PASS
#define CONFIG_SSH_PASSWORD             CONFIG_CYBERSPACE_SSH_PASSWORD
#endif //CONFIG_CYBERSPACE_AUTH_PASS
#define CYBERSPACE_RECV_BUF_LEN         CONFIG_CYBERSPACE_RECV_BUF_LEN          // 2x80 + 2 terminating \r\n
#define TAG_CYBERSPACE                  CONFIG_TAG_CYBERSPACE
#define CYBERSPACE_RETRY_STOP_CMD_EXEC  CONFIG_CYBERSPACE_RETRY_STOP_CMD_EXEC
#define CYBERSPACE_RETRY_START_CMD_EXEC CONFIG_CYBERSPACE_RETRY_START_CMD_EXEC  

#if CONFIG_MOD_CYBERSPACE_LOG_LEVEL == 0
#define MOD_CYBERSPACE_LOG_LEVEL esp_log_level_t::ESP_LOG_NONE
#elif CONFIG_MOD_CYBERSPACE_LOG_LEVEL == 1
#define MOD_CYBERSPACE_LOG_LEVEL esp_log_level_t::ESP_LOG_ERROR
#elif CONFIG_MOD_CYBERSPACE_LOG_LEVEL == 2
#define MOD_CYBERSPACE_LOG_LEVEL esp_log_level_t::ESP_LOG_WARN
#elif CONFIG_MOD_CYBERSPACE_LOG_LEVEL == 3
#define MOD_CYBERSPACE_LOG_LEVEL esp_log_level_t::ESP_LOG_INFO
#elif CONFIG_MOD_CYBERSPACE_LOG_LEVEL == 4
#define MOD_CYBERSPACE_LOG_LEVEL esp_log_level_t::ESP_LOG_DEBUG
#elif CONFIG_MOD_CYBERSPACE_LOG_LEVEL == 5
#define MOD_CYBERSPACE_LOG_LEVEL esp_log_level_t::ESP_LOG_VERBOSE
#endif //CONFIG_MOD_CYBERSPACE_LOG_LEVEL


////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////
#define MAX_COMMAND_LENGTH              CONSOLE_MAX_COMMAND_LINE_LENGTH


////////////////////////////////////////////////////////////////////////////////
// Macros
////////////////////////////////////////////////////////////////////////////////
#ifdef CONFIG_DISPLAY_SUPPORT
#define LCD_PRINT_INIT_LCD_BOUNDS(b)            rect16 _lcd_bounds = b
#define LCD_PRINT_INIT_BANNER_HEIGHT(bh)        int16_t _banner_height = bh
#define LCD_PRINT_INIT_MESSAGE_AREA_WIDTH(mw)   int16_t _message_width = mw
#define LCD_PRINT_INIT_STATUS_AREA_WIDTH(sw)    int16_t _status_width = sw
#define LCD_PRINT_INIT_TOP_OFFSET(o)            int16_t _top_offset = o 
#define LCD_PRINT_INIT_STAT_FAIL_MSG(mf)        const char *_fail_msg = mf
#define LCD_PRINT_INIT_STAT_OK_MSG(mo)          const char *_ok_msg = mo
#define LCD_PRINT_HEADER(d, m)                  _lcd_print_header(d, m, _lcd_bounds)
#define LCD_PRINT_MSG(d, m)                     _lcd_print_msg(d, m, rect16(_lcd_bounds.x1, _lcd_bounds.y1 + _banner_height, _lcd_bounds.x2 - _status_width, _lcd_bounds.y2), _top_offset)
#define LCD_PRINT_STAT_OK(d)                    _lcd_print_msg(d, _ok_msg, rect16(_lcd_bounds.x1 + _message_width, _lcd_bounds.y1 + _banner_height, _lcd_bounds.x2, _lcd_bounds.y2), _top_offset, color<typename Destination::pixel_type>::green)
#define LCD_PRINT_STAT_FAIL(d)                  _lcd_print_msg(d, _fail_msg, rect16(_lcd_bounds.x1 + _message_width, _lcd_bounds.y1 + _banner_height, _lcd_bounds.x2, _lcd_bounds.y2), _top_offset, color<typename Destination::pixel_type>::red)
#define LCD_ADJUST_DRAW_OFFSET_OK(m)            _lcd_adjust_draw_offset((ssize16)rect16(_lcd_bounds.x1, _lcd_bounds.y1 + _banner_height, _lcd_bounds.x2 - _status_width, _lcd_bounds.y2).dimensions(), m, (ssize16)rect16(_lcd_bounds.x1 + _message_width, _lcd_bounds.y1 + _banner_height, _lcd_bounds.x2, _lcd_bounds.y2).dimensions(), _ok_msg, &_top_offset) 
#define LCD_ADJUST_DRAW_OFFSET_FAIL(m)          _lcd_adjust_draw_offset((ssize16)rect16(_lcd_bounds.x1, _lcd_bounds.y1 + _banner_height, _lcd_bounds.x2 - _status_width, _lcd_bounds.y2).dimensions(), m, (ssize16)rect16(_lcd_bounds.x1 + _message_width, _lcd_bounds.y1 + _banner_height, _lcd_bounds.x2, _lcd_bounds.y2).dimensions(), _fail_msg, &_top_offset) 
#else //CONFIG_DISPLAY_SUPPORT
#define LCD_PRINT_INIT_LCD_BOUNDS(b)            ((void)0)
#define LCD_PRINT_INIT_BANNER_HEIGHT(bh)        ((void)0)
#define LCD_PRINT_INIT_MESSAGE_AREA_WIDTH(mw)   ((void)0)
#define LCD_PRINT_INIT_STATUS_AREA_WIDTH(sw)    ((void)0)
#define LCD_PRINT_INIT_TOP_OFFSET(o)            ((void)0)
#define LCD_PRINT_INIT_STAT_FAIL_MSG(mf)        ((void)0)
#define LCD_PRINT_INIT_STAT_OK_MSG(mo)          ((void)0)
#define LCD_PRINT_HEADER(d, m)                  ((void)0)
#define LCD_PRINT_MSG(d, m)                     ((void)0)
#define LCD_PRINT_STAT_OK(d)                    ((void)0)
#define LCD_PRINT_STAT_FAIL(d)                  ((void)0)
#define LCD_ADJUST_DRAW_OFFSET_OK(m)            ((void)0) 
#define LCD_ADJUST_DRAW_OFFSET_FAIL(m)          ((coid)0)
#endif //CONFIG_DISPLAY_SUPPORT

////////////////////////////////////////////////////////////////////////////////
// Error handling
////////////////////////////////////////////////////////////////////////////////

/** @typedef The error type for any led related error */
typedef BaseType_t cyberspace_err_t;

#define CYBERSPACE_FAIL                         -1                              /**< Generic failure */
#define CYBERSPACE_OK                           0x000                           /**< Success */
#define CYBERSPACE_SSH_INIT_FAIL                0x010                           /**< Ssh initialisation fail */
#define CYBERSPACE_GETHOSTBYNAME_FAIL           0x011                           /**< Get host by name failed */
#define CYBERSPACE_CREATE_SOCKET_FAIL           0x012                           /**< Socket creation failed */
#define CYBERSPACE_CONNECT_FAIL                 0x013                           /**< Connection failed */
#define CYBERSPACE_SSH_SESSION_FAIL             0x014                           /**< SSH session failed */
#define CYBERSPACE_SSH_HANDSHAKE_FAIL           0x015                           /**< SSH handshake failed */
#define CYBERSPACE_SSH_CHANNEL_OPEN_FAIL        0x020                           /**< Failed to open channel */
#define CYBERSPACE_SSH_CHANNEL_READ_FAIL        0x021                           /**< Failed to read from channel */
#define CYBERSPACE_SSH_AUTH_FAIL                0x030                           /**< Authentication failed */


namespace bcd_cyberspace {

    ////////////////////////////////////////////////////////////////////////////
    // Function prototypes
    ////////////////////////////////////////////////////////////////////////////
    int waitsocket(int socket_fd, LIBSSH2_SESSION *session);
    inline void _terminate_ssh_session(LIBSSH2_CHANNEL *channel, LIBSSH2_SESSION *session, int socket_fd, cyberspace_err_t reason);
    inline cyberspace_err_t printf_channel_content_or_terminate(LIBSSH2_CHANNEL *channel, LIBSSH2_SESSION *session, int socket_fd, bool *printed_something);
    void _lcd_adjust_draw_offset(ssize16 msg_area_dimensions, const char *m, ssize16 stat_area_dimensions, const char *s, int16_t *o);
    cyberspace_err_t _establish_ssh_channel(LIBSSH2_SESSION **session, LIBSSH2_CHANNEL **channel, int *sock);

    template<typename Destination>
    void main_loop(LIBSSH2_CHANNEL *channel, LIBSSH2_SESSION *session, int sock, QueueHandle_t qData, QueueHandle_t qSignal, Destination *lcd);
   
    ////////////////////////////////////////////////////////////////////////////
    // Template Implementations
    ////////////////////////////////////////////////////////////////////////////
#ifdef CONFIG_DISPLAY_SUPPORT
    template<typename Destination>
    void _lcd_print_header(Destination *lcd, const char *header, rect16 header_area, typename Destination::pixel_type color = color<typename Destination::pixel_type>::crimson) {
        const font &font = Bm437_Acer_VGA_8x8_FON;
        draw::text(*lcd,
            header_area,
            header,
            font,
            color);  
    }

    template<typename Destination>
    void _lcd_print_msg(Destination *lcd, const char *msg, rect16 msg_area, uint16_t top_offset, typename Destination::pixel_type color = color<typename Destination::pixel_type>::steel_blue) {
        const font &font = Bm437_Acer_VGA_8x8_FON;
        draw::text(*lcd,
                rect16(
                    msg_area.x1,
                    msg_area.y1 + top_offset,
                    msg_area.x2,
                    msg_area.y2
                ),
                msg,
                font,
                color);
    };
   
#endif //CONFIG_DISPLAY_SUPPORT

    /**
     * @brief 
     * 
     *
     * @tparam Destination
     * @param session 
     * @param qData 
     * @param qSignal 
     * @param lcd 
     */
    template<typename Destination>
    void main_loop(LIBSSH2_CHANNEL *channel, LIBSSH2_SESSION *session, int sock, QueueHandle_t qData, QueueHandle_t qSignal, Destination *lcd) {

        const font &font = Bm437_Acer_VGA_8x8_FON;
        ledDriver &led = ledDriver::getInstance();

        // This is our main loop. In this loop we will try to read the channel,
        // check if we received a new command (and potentially wait for a given
        // time to check if new content was transfered to us).
        espconsole::consoleData buf;
        BaseType_t xStatus;
        for(;;) {

            // Read command
            xStatus = xQueueReceive(qData, &buf, 0);
            if(xStatus == pdPASS) {
                // Process the command
                ESP_LOGV(TAG_CYBERSPACE, "Command received was: %s", buf.raw_content);

                if(strcasecmp(buf.raw_content, "quit") == 0) {
                    // if we receive a quit command, we terminate the session
                    // TODO maybe let the server send us a quit confirmation first
                    break;
                } else {
                    // forward the command to the ssh server
                    char ssh_cmd[MAX_COMMAND_LENGTH + 4];
                    strcpy(ssh_cmd, buf.raw_content);
                    strcat(ssh_cmd, "\r\n"); // TODO strncat maybe?
                    libssh2_channel_write(channel, ssh_cmd, strlen(ssh_cmd));
                }
            }
    
            // Read response and output to console
            bool printed_something;
            cyberspace_err_t err = printf_channel_content_or_terminate(channel, session, sock, &printed_something);
            if(err != CYBERSPACE_OK) {
                // There was an error, abort
                break;
            }
            if(printed_something) {
                // We did receive something
            }
        }

        
    }

    /**
     * @brief The main function of the cyberspace module
     * 
     * @tparam Destination The typename of the display
     * @param param an array of void pointers with the first pointer pointing to
     *          the lcd and the second one pointing to the handle for the 
     *          command task.
     * @return int 0 on success.
     */
    template<typename Destination>
    int sshConnectFunction(void *param) {

        // Set log level
        esp_log_level_set(TAG_CYBERSPACE, MOD_CYBERSPACE_LOG_LEVEL);

        Destination *lcd = (Destination *)(((void **)param)[0]);
        const font &font = Bm437_Acer_VGA_8x8_FON;
        espwifi::wifiController wifi;
        ledDriver &led = ledDriver::getInstance();
        espconsole::consoleController& console = 
            espconsole::consoleController::getInstance();

        cyberspace_err_t return_code = ESP_OK;

        

        // Check if we have a connection first
#ifdef CONFIG_DISPLAY_SUPPORT  
        // Messages and their sizes
        //                                          "12345678901234567890"
        lcd->clear(lcd->bounds());
        const char *banner_msg                  =   "Entering cyberspace:"
                                                    "~~~~~~~~~~~~~~~~~~~~";
        const char *net_connection_msg          =   "Net. connection ";
        const char *data_queue_creation_msg     =   "Data queue      ";
        const char *signal_queue_creation_msg   =   "Signal queue    ";
        const char *exclusive_console_fwd_msg   =   "Excl. cons. fwd.";
        const char *stop_cmd_exec_msg           =   "Stop. cmd. exec.";
        const char *srv_connection_msg          =   "Srv. connection ";
        const char *ok_msg                      =   " OK";
        const char *fail_msg                    =   "FAIL";  
#endif // CONFIG_DISPLAY_SUPPORT

        LCD_PRINT_INIT_LCD_BOUNDS((rect16)lcd->bounds());
        LCD_PRINT_INIT_BANNER_HEIGHT(font.measure_text((ssize16)lcd->dimensions(), banner_msg).height);
        LCD_PRINT_INIT_MESSAGE_AREA_WIDTH(lcd->dimensions().width - font.measure_text((ssize16)lcd->dimensions(), fail_msg).width);
        LCD_PRINT_INIT_STATUS_AREA_WIDTH(font.measure_text((ssize16)lcd->dimensions(), fail_msg).width);
        LCD_PRINT_INIT_TOP_OFFSET(8);
        LCD_PRINT_INIT_STAT_FAIL_MSG(fail_msg);
        LCD_PRINT_INIT_STAT_OK_MSG(ok_msg);
       
        // Draw banner on LCD
        //
        // First lets draw the banner.
        LCD_PRINT_HEADER(lcd, banner_msg);

        // WiFi connection
        //
        // First we check if the wifi is connected. If no connection is present
        // we abort.
        LCD_PRINT_MSG(lcd, net_connection_msg);
        espwifi::wifiController::state_e wifi_state = wifi.GetState();
        switch(wifi_state) {
            case espwifi::wifiController::state_e::CONNECTED:
                ESP_LOGI(TAG_CYBERSPACE, "Connected to network."); 
                LCD_PRINT_STAT_OK(lcd);
                LCD_ADJUST_DRAW_OFFSET_OK(net_connection_msg);
                break;
                
            default:
                ESP_LOGE(TAG_CYBERSPACE, "Not connected to network.");
                LCD_PRINT_STAT_FAIL(lcd); 
                LCD_ADJUST_DRAW_OFFSET_FAIL(net_connection_msg);      
                vTaskDelay(pdMS_TO_TICKS(3000));
                return -1;
        }

        // Console configuration
        //
        // Create a data queue, a signal queue and register as exclusive forward 
        // client. We do not want user input to end up anywhere else.
        LCD_PRINT_MSG(lcd, data_queue_creation_msg);
        QueueHandle_t qData =                                                   // TODO size
            xQueueCreate( 1, sizeof(espconsole::consoleData) );                
        if(qData == NULL) {
            LCD_PRINT_STAT_FAIL(lcd);
            LCD_ADJUST_DRAW_OFFSET_FAIL(data_queue_creation_msg);
            ESP_LOGE(TAG_CYBERSPACE, "Could not create data queue.");
            return -1;
        }

        LCD_PRINT_STAT_OK(lcd);
        LCD_ADJUST_DRAW_OFFSET_OK(data_queue_creation_msg);
        
        // Create signal queue
        LCD_PRINT_MSG(lcd, signal_queue_creation_msg);
        QueueHandle_t qSignal =                                                 // TODO size
            xQueueCreate( 1, sizeof(espconsole::console_signal));               
        if(qSignal == NULL) {
            LCD_PRINT_STAT_FAIL(lcd);
            LCD_ADJUST_DRAW_OFFSET_FAIL(data_queue_creation_msg);
            ESP_LOGE(TAG_CYBERSPACE, "Could not create signal queue.");
            vQueueDelete(qData);
            return -1;
        }
        LCD_PRINT_STAT_OK(lcd);
        LCD_ADJUST_DRAW_OFFSET_OK(data_queue_creation_msg);
        
        // Become an exclusive console forwarder
        LCD_PRINT_MSG(lcd, exclusive_console_fwd_msg);
        espconsole::consoleForwarderHandle_t hForwarder;
        console_err_t cerr = 
            console.requestInputRedirect(qData, qSignal, &hForwarder, true);
        if(cerr != CONSOLE_OK) {
            ESP_LOGE(TAG_CYBERSPACE, "Failed to register as exclusive console "
                "forwarder.");
            LCD_PRINT_STAT_FAIL(lcd);
            LCD_ADJUST_DRAW_OFFSET_FAIL(data_queue_creation_msg);
        }
        else {
            ESP_LOGI(TAG_CYBERSPACE,"Successfully registered as exclusive forwarder!");
            LCD_PRINT_STAT_OK(lcd);
            LCD_ADJUST_DRAW_OFFSET_OK(data_queue_creation_msg);
        }
       
        // Try to stop command execution
        // 
        // We retry at most retry_count times. If we do not succeed stopping the
        // command execution, we will continue anyways. 
        LCD_PRINT_MSG(lcd, stop_cmd_exec_msg);
        int retry_count = CYBERSPACE_RETRY_STOP_CMD_EXEC;                                                    // TODO make configurable.
        bool success = false;
        do {
            vTaskDelay(pdMS_TO_TICKS(500));
            if(console.stopCommandExecution(&hForwarder) == CONSOLE_OK) {
                success = true;
                break;
            }
            retry_count--;
        } while(retry_count > 0);
        if(!success) {
            ESP_LOGW(
                TAG_CYBERSPACE, 
                "Could not disable input redirect. Type commands at own risk!");
            LCD_PRINT_STAT_FAIL(lcd);
            LCD_ADJUST_DRAW_OFFSET_FAIL(data_queue_creation_msg);
        } 
        else {
            ESP_LOGI(TAG_CYBERSPACE, "Successfully disabled command execution.");
            LCD_PRINT_STAT_OK(lcd);
            LCD_ADJUST_DRAW_OFFSET_OK(data_queue_creation_msg);
        }

        // Establish the ssh session
        int sock;
        LIBSSH2_SESSION *session;
        LIBSSH2_CHANNEL *channel;

        LCD_PRINT_MSG(lcd, srv_connection_msg);
        return_code = _establish_ssh_channel(&session, &channel, &sock);
        if(return_code == ESP_OK) {
            LCD_PRINT_STAT_OK(lcd);
            LCD_ADJUST_DRAW_OFFSET_OK(srv_connection_msg);
        } else {
            LCD_PRINT_STAT_FAIL(lcd);
            LCD_ADJUST_DRAW_OFFSET_FAIL(srv_connection_msg);
        }
        switch(return_code) {
            case CYBERSPACE_SSH_INIT_FAIL:
                ESP_LOGE(TAG_CYBERSPACE, "libssh2 initialization failed (%d)", return_code);
                break;

            case CYBERSPACE_GETHOSTBYNAME_FAIL:
                ESP_LOGE(TAG_CYBERSPACE, "Gethostbyname failed.");
                break;

            case CYBERSPACE_CREATE_SOCKET_FAIL:
                ESP_LOGE(TAG_CYBERSPACE, "Failed to create socket for ssh conneciton.");
                break;

            case CYBERSPACE_CONNECT_FAIL:
                ESP_LOGE(TAG_CYBERSPACE, "Failed to connect.");
                break;

            case CYBERSPACE_SSH_SESSION_FAIL:
                ESP_LOGE(TAG_CYBERSPACE, "Failed to initialise ssh session.");
                break;

            case CYBERSPACE_SSH_HANDSHAKE_FAIL:
                ESP_LOGE(TAG_CYBERSPACE, "SSH handshake failed.");
                break;

            case CYBERSPACE_SSH_AUTH_FAIL:
                ESP_LOGE(TAG_CYBERSPACE, "SSH authentication failed.");
                break;

            case CYBERSPACE_OK:
                ESP_LOGI(TAG_CYBERSPACE, "SSH session established.");
                break;

            default:
                ESP_LOGW(TAG_CYBERSPACE, "Unknown state reached for ssh session.");
                break;
        }

        // Start the main game loop
        //
        // If the loop terminates, we tear down the ssh session.
        if(return_code == ESP_OK) {
            bcd_cyberspace::main_loop(channel, session, sock, qData, qSignal, lcd);

            // Tear the ssh session down
            _terminate_ssh_session(channel, session, sock, CYBERSPACE_OK);
        }  


#ifdef CONFIG_DISPLAY_SUPPORT 
            lcd->clear(lcd->bounds());
            const char *text2 = "Exiting Cyberspace...\r\n";
            draw::text(*lcd,
                    (srect16)lcd->bounds(),
                    text2,
                    font,
                    color<typename Destination::pixel_type>::green);   
#endif // CONFIG_DISPLAY_SUPPORT

        // Reenable command exectuion
        retry_count = CYBERSPACE_RETRY_START_CMD_EXEC;
        success = false;
        do {
            vTaskDelay(pdMS_TO_TICKS(500));
            if(console.enableCommandExecution(&hForwarder) == CONSOLE_OK) {
                success = true;
                break;
            }
            retry_count--;
        } while(retry_count > 0);
        if(!success) {
            ESP_LOGW(TAG_CYBERSPACE, "Could not enable command execution.");
        }


        // Deregister console forward
        //
        // Note: We need to succeed, otherwise the programm will crash. Therefore
        //          we try forever.
        // TODO - display message that we are trying to quit
        do {
            if(console.cancelInputRedirect(&hForwarder) == CONSOLE_OK) {
                break;
            }
            vTaskDelay(pdMS_TO_TICKS(200));
        } while(true);
        vQueueDelete(qSignal);
        vQueueDelete(qData);
        vTaskDelay(pdMS_TO_TICKS(2000));    // TODO verify if needed
        return 0;
    }

    /**
     * @brief Terminate the ssh session
     * 
     *
     * @param session 
     * @param socket_fd 
     */
    inline void _terminate_ssh_session(LIBSSH2_CHANNEL *channel, LIBSSH2_SESSION *session, int socket_fd, cyberspace_err_t reason) {
        
        char *reason_msg;
        if(reason == CYBERSPACE_OK) {
            reason_msg=(char *)"Terminating session due to shutdown";
        } else {
            reason_msg=(char *)"Terminating session due to failure";
        } 
        int err = 0;
        int exitcode = 127;
        char *exitsignal = (char *)"none";

        switch(reason) {
            case CYBERSPACE_OK:
            case CYBERSPACE_SSH_CHANNEL_READ_FAIL:
                ESP_LOGD(TAG_CYBERSPACE, "Closing channel...");

           
                while((err = libssh2_channel_close(channel)) == LIBSSH2_ERROR_EAGAIN) {
                    ESP_LOGD(TAG_CYBERSPACE, "Waiting on socket for channel close.");
                    waitsocket(socket_fd, session);
                }

                if(err == 0) {
                    exitcode = libssh2_channel_get_exit_status(channel);
                    libssh2_channel_get_exit_signal(channel, &exitsignal,
                                                    NULL, NULL, NULL, NULL, NULL);
                } else {
                    ESP_LOGW(TAG_CYBERSPACE, "libssh2_channel_close failed: %d. Moving on.", err);
                }

                if(exitsignal) {
                    ESP_LOGI(TAG_CYBERSPACE, "EXIT: %d, SIGNAL: %s", exitcode, exitsignal);
                } else {
                    ESP_LOGI(TAG_CYBERSPACE, "EXIT: %d", exitcode);
                }

                libssh2_channel_free(channel);
                [[fallthrough]];

            case CYBERSPACE_SSH_HANDSHAKE_FAIL:
            case CYBERSPACE_SSH_AUTH_FAIL:
            case CYBERSPACE_SSH_CHANNEL_OPEN_FAIL:
                // Close ssh session and free resources
                ESP_LOGD(TAG_CYBERSPACE, "%s", reason_msg);
                libssh2_session_disconnect(session, reason_msg);
                libssh2_session_free(session);
                [[fallthrough]];

            case CYBERSPACE_SSH_SESSION_FAIL:
            case CYBERSPACE_CONNECT_FAIL:
                // Close socket
                close(socket_fd);
                [[fallthrough]];

            case CYBERSPACE_CREATE_SOCKET_FAIL:
            case CYBERSPACE_GETHOSTBYNAME_FAIL:
                // Free libssh2 resources
                libssh2_exit();
                [[fallthrough]];

            case CYBERSPACE_SSH_INIT_FAIL:
            default:
                break;
        }
        return;
    }

    /**
     * @brief Read from an ssh channel until the channel is empty
     * 
     * @param session 
     * @param channel 
     * @param socket_fd 
     * @return true, if something was read and output to the console, false otherwise
     */
    inline cyberspace_err_t printf_channel_content_or_terminate(LIBSSH2_CHANNEL *channel, LIBSSH2_SESSION *session, int socket_fd, bool *printed_something) {

        int rc;
        char buffer[CYBERSPACE_RECV_BUF_LEN];
        *printed_something = false;
        
        // Check if we need to clear blocking EAGAIN
        if(libssh2_session_last_error(session, NULL, NULL, 0) ==
            LIBSSH2_ERROR_EAGAIN) {
            ESP_LOGV(TAG_CYBERSPACE, "Clearing EAGAIN");
            waitsocket(socket_fd, session);
        }

        // Read until we would block (which means there is no more data to read)
        while(1) {
            rc = libssh2_channel_read(channel, buffer, CYBERSPACE_RECV_BUF_LEN-1);
            if(rc > 0) {
                buffer[rc] = '\0';
                printf("%s", buffer);
                *printed_something = true;
            } else if (rc == LIBSSH2_ERROR_EAGAIN) {
                ESP_LOGV(TAG_CYBERSPACE, "Channel would block (EAGAIN)");
                break;
            } else {
                break;
            }		
        }

        // Check if we encountered an error reading the channel.
        if(rc < 0 && rc != LIBSSH2_ERROR_EAGAIN) {
            char *error_message = NULL;
            libssh2_session_last_error(session, &error_message, NULL, 0);
            ESP_LOGE(TAG_CYBERSPACE, "Error reading from channel (%s): Terminating.", error_message);
            return CYBERSPACE_SSH_CHANNEL_READ_FAIL;
            
        } 

        ESP_LOGV(TAG_CYBERSPACE, "Finished reading channel for good.");
        return CYBERSPACE_OK;
    }

}
#endif // BCD_MODULE_SSH_CONNECT_H