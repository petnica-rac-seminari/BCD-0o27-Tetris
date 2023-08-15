/** 
 * @file        ch405labs_esp_wifi.hpp
 * @author      Florian Schuetz (fschuetz@ieee.org)
 * @brief       Functions to manage wifi connections on the esp32.
 * @version     0.1
 * @date        2023-03-10
 * @copyright   Copyright (c) 2023, Florian Schuetz, released under MIT license
 *
 * This class implements function to manage wifi connections on the esp32. 
 * The library is inspired by 
 * https://embeddedtutorials.com/eps32/esp32-wifi-cpp-esp-idf-station/
 *
 * 
 * @todo esp_err_t esp_wifi_clear_ap_list(void) Clear AP list found in last scan.
 *          When the obtained ap list fails,bss info must be cleared,otherwise 
 *          it may cause memory leakage.
 */
#pragma once

#include <cstring>
#include <iostream>
#include <iomanip>
#include <mutex>
#include <iostream>
#include <fstream>

#include "esp_wifi.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include <vector> // TODO remove debug

////////////////////////////////////////////////////////////////////////////////
// Menuconfig options
////////////////////////////////////////////////////////////////////////////////
#define WIFI_SCAN_LIST_SIZE         CONFIG_WIFI_SCAN_LIST_SIZE
#define WIFI_AP_SAVE_FILE           CONFIG_WIFI_AP_SAVE_FILE

#if CONFIG_WIFI_LOG_LEVEL == 0
#define WIFI_LOG_LEVEL esp_log_level_t::ESP_LOG_NONE
#elif CONFIG_WIFI_LOG_LEVEL == 1
#define WIFI_LOG_LEVEL esp_log_level_t::ESP_LOG_ERROR
#elif CONFIG_WIFI_LOG_LEVEL == 2
#define WIFI_LOG_LEVEL esp_log_level_t::ESP_LOG_WARN
#elif CONFIG_WIFI_LOG_LEVEL == 3
#define WIFI_LOG_LEVEL esp_log_level_t::ESP_LOG_INFO
#elif CONFIG_WIFI_LOG_LEVEL == 4
#define WIFI_LOG_LEVEL esp_log_level_t::ESP_LOG_DEBUG
#elif CONFIG_WIFI_LOG_LEVEL == 5
#define WIFI_LOG_LEVEL esp_log_level_t::ESP_LOG_VERBOSE
#endif //CONFIG_WIFI_LOG_LEVEL

#if CONFIG_WIFI_DRIVER_LOG_LEVEL == 0
#define WIFI_DRIVER_LOG_LEVEL esp_log_level_t::ESP_LOG_NONE
#elif CONFIG_WIFI_DRIVER_LOG_LEVEL == 1
#define WIFI_DRIVER_LOG_LEVEL esp_log_level_t::ESP_LOG_ERROR
#elif CONFIG_WIFI_DRIVER_LOG_LEVEL == 2
#define WIFI_DRIVER_LOG_LEVEL esp_log_level_t::ESP_LOG_WARN
#elif CONFIG_WIFI_DRIVER_LOG_LEVEL == 3
#define WIFI_DRIVER_LOG_LEVEL esp_log_level_t::ESP_LOG_INFO
#elif CONFIG_WIFI_DRIVER_LOG_LEVEL == 4
#define WIFI_DRIVER_LOG_LEVEL esp_log_level_t::ESP_LOG_DEBUG
#elif CONFIG_WIFI_DRIVER_LOG_LEVEL == 5
#define WIFI_DRIVER_LOG_LEVEL esp_log_level_t::ESP_LOG_VERBOSE
#endif //CONFIG_DRIVER_WIFI_LOG_LEVEL

////////////////////////////////////////////////////////////////////////////////
// Error handling
////////////////////////////////////////////////////////////////////////////////

/** @typedef The error type for any wifi related error */
typedef BaseType_t wifi_err_t;

#define WIFI_FAIL                       -1      /**< Generic failure */
#define WIFI_OK                         0x000   /**< Success */
#define WIFI_INVALID_ARG                0x010   /**< Invalid argument */
#define WIFI_NO_MEM                     0x011   /**< Not enough free memory */
#define WIFI_NVS_ERR                    0x012   /**< WiFi internal nvs error */
#define WIFI_PERSIST_FAIL               0x013   /**< Failed to persist data */
#define WIFI_FILE_OPEN_FAIL             0x014   /**< Failed to open file */
#define WIFI_NOT_INIT                   0x020   /**< WiFi not initialised */
#define WIFI_NOT_STARTED                0x021   /**< WiFi not started */
#define WIFI_NOT_CONNECTED              0x022   /**< WiFi station not connected */
#define WIFI_MODE_ERR                   0x023   /**< Mode error */
#define WIFI_CONN_ERR                   0x024   /**< WiFi internal error */
#define WIFI_IF_INVALID                 0x025   /**< WiFi interface invalid */
#define WIFI_PWD_INVALID                0x026   /**< WiFi password invalid */
#define WIFI_CONNECTION_FAIL            0x030   /**< Failed to establish connection */
#define WIFI_DISCONNECT_FAIL            0x031   /**< Failed to disconnect */

////////////////////////////////////////////////////////////////////////////////
// Debugging
////////////////////////////////////////////////////////////////////////////////
static const char TAG_WIFI[] = CONFIG_TAG_WIFI;                                 /**< TAG for ESP_LOGX macro. */
static const char TAG_WIFI_DRIVER[] = CONFIG_TAG_WIFI_DRIVER;                   /**< TAG for EXP_LOGX macro.*/


////////////////////////////////////////////////////////////////////////////////
// Constants
////////////////////////////////////////////////////////////////////////////////
#define SSID_MAX_LEN    32          /**< Max lenght of an ssid. Note: Only needs 0 termination if less than 32 characters */
#define PASS_MAX_LEN    64          /**< Max lenght of a password. Note: Only needs 0 termination if less than 64 characters */    
#define BSSID_LEN        6          /**< Length of the BSSID */


namespace espwifi {
    /**
     * @brief A class that describes a wifi access point
     * 
     * This class describes an access point. It can be used to store access 
     * point information or be passed to the wifi manager to, for example,
     * establish a connection.
     * 
     * @todo Copy and move constructor
     * 
     */
    class accessPoint {
        private:
            wifi_ap_record_t _record{};
            wifi_err_t _lasterror{WIFI_OK};
            bool _valid{false};

            const char *_authmodeAsString() const;
            const char *_pairwiseCipherAsString() const;
            const char *_groupCipherAsString() const;

        public:
            /**
             * @brief Constructor to create empty access point object.
             */
            accessPoint(void);

            /**
             * @brief Constructor to create an access point object from a
             *          wifi_ap_record_t struct.
             * 
             * @param record The record from which the ap object is to be created.
             */
            accessPoint(wifi_ap_record_t record);
            
            /**
             * @brief Get the SSID of the access point object.
             * 
             *
             * @return The name of the access point or NULL if the access point
             *          object has not been properly initialised.
             */
            uint8_t *getSsid();

            /**
             * @brief Set the access point ssid
             * 
             * @param ssid The SSID as a reference to a null terminated 
             *              character string.
             * @return WIFI_OK on success, WIFI_INVALID_ARG if the name for the
             *             access point is not valid.
             */
            wifi_err_t setSsid(const char &ssid);
            
            /*
             * Functions for output
             */
            friend std::ostream& operator<<(std::ostream& os, const accessPoint& ap);
            friend std::ofstream& operator<<(std::ofstream& os, const accessPoint& ap);
            std::ofstream& serialize(std::ofstream &os) const;
            std::ifstream& deserialize(std::ifstream &is);
    };

    class accessPointList {
        private:
            // TODO make proper with dynamic memory
            accessPoint _ap_list[WIFI_SCAN_LIST_SIZE]{};
            int _current_elem{0};
            int _num_elem{0};

            template <typename PointerType> class _iterator {
                // TODO make the iterator constant
                public: 
                    using iterator_category = std::forward_iterator_tag;
                    using difference_type   = std::ptrdiff_t;
                    using value_type        = PointerType;
                    using pointer           = PointerType*;  
                    using reference         = PointerType&;  

                    _iterator(pointer ptr) : m_ptr(ptr) {}

                    reference operator*() const { return *m_ptr; }
                    pointer operator->() { return m_ptr; }

                    // Prefix increment
                    _iterator& operator++() { m_ptr++; return *this; }  

                    // Postfix increment
                    _iterator operator++(int) { _iterator tmp = *this; ++(*this); return tmp; }

                    friend bool operator== (const _iterator& a, const _iterator& b) { return a.m_ptr == b.m_ptr; };
                    friend bool operator!= (const _iterator& a, const _iterator& b) { return a.m_ptr != b.m_ptr; };   

                private:
                    PointerType *m_ptr;
            };

        public:
            typedef _iterator<accessPoint> Iterator;
            typedef _iterator<const accessPoint> ConstantIterator;

            Iterator begin() { return Iterator(&_ap_list[0]); }
            Iterator end()   { return Iterator(&_ap_list[_num_elem+1]); } 

            ConstantIterator cbegin() const { return ConstantIterator(&_ap_list[0]); }
            ConstantIterator cend()   const { return ConstantIterator(&_ap_list[_num_elem+1]); }

            wifi_err_t addAccessPoint(accessPoint ap) {
                //TODO proper error values
                if(_current_elem < WIFI_SCAN_LIST_SIZE) {
                    _ap_list[_current_elem] = ap;
                    _current_elem++;
                    _num_elem++;
                    return WIFI_OK; 
                } else {
                    return WIFI_FAIL;
                }
            }

            void clear() {
                _current_elem = 0;
                _num_elem = 0;
            }
    };
    
    /**
     * @brief Configuration for the Wifi interface
     * 
     * An object capturing a configuration for the Wifi interface. The 
     * configuraiton can be stored using the serialisation function.
     * 
     * TODO:
     *  - implement rule of three / five
     *  - make authmode an own class to enable printing stings
     */
    class wifiStaConfig {
        private:
            const uint8_t _version = 0x02;              /**< Version of the format. Update if serialisation changes */
            wifi_sta_config_t _config = {
                .ssid = {0},
                .password = {0},
                .scan_method = wifi_scan_method_t::WIFI_ALL_CHANNEL_SCAN,
                .bssid_set = false,
                .bssid = {0},
                .channel = 0,
                .listen_interval = 0,
                .sort_method = wifi_sort_method_t::WIFI_CONNECT_AP_BY_SECURITY,
                .threshold {
                    .rssi = 0,
                    .authmode = wifi_auth_mode_t::WIFI_AUTH_OPEN
                },
                .pmf_cfg {
                    .capable = true,
                    .required = false
                },
                .rm_enabled = true,
                .btm_enabled = true,
                .mbo_enabled = true,
                .ft_enabled = 1,
                .owe_enabled = 1,
                .transition_disable = 1,
                .reserved = 0,
                .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
                .failure_retry_cnt = 3,
            };

            void _authmodeToString(wifi_auth_mode_t, char *, size_t);

        public:
            wifiStaConfig();
            explicit wifiStaConfig(const wifi_sta_config_t &cfg);
            wifiStaConfig &operator=(const wifiStaConfig &rhs);

            /**
             * @brief Sets the SSID of the access point to connect to
             * 
             * This function silently truncates the provided ssid to 32 
             * characters if required.
             * 
             * Note: The ssid of an access point can be a maximum of 32 
             * characters. If the ssid is less than 32 characters it needs to
             * be 0 terminated, if it is 32 characters it is not terminated.
             * 
             * @param ssid The name of the ssid
             * @return WIFI_OK on success.
             */
            wifi_err_t setSsid(const char *ssid);
            wifi_err_t getSsid(char *ssid, size_t length);
            wifi_err_t setPassword(const char *pass);
            wifi_err_t getPassword(char *pass, size_t length);
            void setScanMethod(wifi_scan_method_t method);
            wifi_scan_method_t getScanMethod();
            wifi_err_t setBssid(uint8_t (&bssid)[6]);
            wifi_err_t getBssid(uint8_t (&bssid)[6]);
            void setEnforceBssid(bool enforce);
            bool getEnforceBssid();
            wifi_err_t setChannel(uint8_t channel);
            uint8_t getChannel();
            void setListenInterval(uint16_t interval);
            uint16_t getListenInterval();
            wifi_err_t setSortMethod(wifi_sort_method_t method);
            wifi_sort_method_t getSortMethod();
            void setScanThreshold(wifi_scan_threshold_t threshold);
            wifi_scan_threshold_t getScanThreshold();
            void setScanThresholdRSSI(uint8_t rssi_threshold);
            uint8_t getScanThresholdRSSI();
            void setScanThresholdAuthMode(wifi_auth_mode_t auth_mode);
            wifi_auth_mode_t getScanThresholdAuthMode();
            void getScanThresholdAuthModeAsString(char *str, size_t len);
            void setPMFConfig(wifi_pmf_config_t cfg);
            wifi_pmf_config_t getPMFConfig();
            void setPMFCapable(bool capable);
            bool getPMFCapable();
            void setPMFRequired(bool required);
            bool getPMFRequired();
            void setRM(bool rm_enabled);
            bool getRM();
            void setBTM(bool btm_enabled);
            bool getBTM();
            void setMBO(bool mbo_enabled);
            bool getMBO();
            void setFTEnabled(bool ft_enabled);
            bool getFTEnabled();
            void setOWEEnabled(bool owe_enabled);
            bool getOWEEnabled();
            void setTransitionDisable(bool transition_disable);
            bool getTransitionDisable();
            void setSaePweH2e(wifi_sae_pwe_method_t sae_pwe_h2e);
            wifi_sae_pwe_method_t getSaePweH2e();
            void setFailureRetryCount(uint8_t failure_retry_count);
            uint8_t getFailureRetryCount();


            std::ofstream &serialize(std::ofstream &os) const;
            std::ifstream &deserialize(std::ifstream &is);

    };

    /**
     * @brief The main class for wifi configruation and operations
     * 
     * The wifiController is implemented as a singleton to directly interface
     * with the wifi controller on the esp32.
     * 
     */
    class wifiController {
    public:
        enum class state_e
        {
            NOT_INITIALIZED,
            INITIALIZED,
            READY_TO_CONNECT,
            CONNECTING,
            WAITING_FOR_IP,
            CONNECTED,
            DISCONNECTED,
            ERROR
        };

    private:
        static accessPointList _ap_list;
        static wifi_err_t _init();
        static bool initialised;
        static wifi_err_t lasterror;
        static wifi_init_config_t _wifi_init_cfg;
        static wifi_config_t _wifi_cfg;

        static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                                       int32_t event_id, void *event_data);
        static void ip_event_handler(void *arg, esp_event_base_t event_base,
                                     int32_t event_id, void *event_data);

        static state_e _state;
        static esp_err_t _get_mac(void);
        static char _mac_addr_cstr[13];
        static std::mutex _mutx;

        // Helpers
        static void print_auth_mode(int authmode);
        static void print_cipher_type(int pairwise_cipher, int group_cipher);


    public:
        wifiController(void);
        explicit wifiController(wifiStaConfig &cfg);
        wifi_err_t init();
        state_e getWifiState();

        void setStaConfig(wifiStaConfig &cfg);
        wifiStaConfig getStaConfig();
        
        void setSsid(const char *);
        void setPassword(const char *);

        wifi_err_t connect(bool =false);
        wifi_err_t disconnect(bool =false);


        constexpr static const state_e &GetState(void) { return _state; }
        constexpr static const char *GetMac(void) { return _mac_addr_cstr; }

        wifi_err_t scanAPs();
        accessPointList getAPList();
        wifi_err_t staGetAPInfo(accessPoint &ap);

        // TODO change to make this taking an accessPoint object and persist it.
        wifi_err_t saveAP(char *name, char *pass);
        wifi_err_t saveConfig(const wifiStaConfig &cfg);
        wifi_err_t loadConfigs(std::vector<wifiStaConfig> &cfg_list);
    }; 

} 