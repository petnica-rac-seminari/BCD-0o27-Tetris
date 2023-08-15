/** 
 * @file        ch405labs_esp_wifi.cpp
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
 * Details see ch405labs_esp_wifi.hpp.
 * 
 * @todo esp_err_t esp_wifi_clear_ap_list(void) Clear AP list found in last scan.
 *          When the obtained ap list fails,bss info must be cleared,otherwise 
 *          it may cause memory leakage.
 */
#include "ch405labs_esp_wifi.hpp"

namespace espwifi
{
    /*
     * Internal helper functions
     */
    inline wifi_err_t translate_esp_err_t(esp_err_t e) {
        switch (e) {
            case ESP_OK:
                return WIFI_OK;
                break;

            case ESP_ERR_NO_MEM:
                return WIFI_NO_MEM;
                break;

            case ESP_ERR_WIFI_MODE:
                return WIFI_MODE_ERR;
                break;
            case ESP_ERR_WIFI_NVS: 
                return WIFI_NVS_ERR;
                break;

            case ESP_ERR_WIFI_NOT_INIT: 
                return WIFI_NOT_INIT;
                break;

            case ESP_ERR_WIFI_CONN: 
                return WIFI_CONN_ERR;
                break;

            case ESP_ERR_WIFI_IF: 
                return WIFI_IF_INVALID;
                break;

            case ESP_ERR_WIFI_PASSWORD:
                return WIFI_PWD_INVALID;
                break;

            case ESP_ERR_WIFI_NOT_STARTED:
                return WIFI_NOT_STARTED;
                break;

            default:
                return WIFI_FAIL;
                break;
        }
    }

    /*
     * accessPoint
     */
    accessPoint::accessPoint(void) {
        this->_valid = false;
    }

    accessPoint::accessPoint(wifi_ap_record_t rec) {
        this->_record = rec;
        this->_valid = true;
    }

    const char *accessPoint::_authmodeAsString() const {
        switch (this->_record.authmode) {
        case WIFI_AUTH_OPEN:
            return "WIFI_AUTH_OPEN";
            break;
        case WIFI_AUTH_WEP:
            return "WIFI_AUTH_WEP";
            break;
        case WIFI_AUTH_WPA_PSK:
            return "WIFI_AUTH_WPA_PSK";
            break;
        case WIFI_AUTH_WPA2_PSK:
            return "WIFI_AUTH_WPA2_PSK";
            break;
        case WIFI_AUTH_WPA_WPA2_PSK:
            return "WIFI_AUTH_WPA_WPA2_PSK";
            break;
        case WIFI_AUTH_WPA2_ENTERPRISE:
            return "WIFI_AUTH_WPA2_ENTERPRISE";
            break;
        case WIFI_AUTH_WPA3_PSK:
            return "WIFI_AUTH_WPA3_PSK";
            break;
        case WIFI_AUTH_WPA2_WPA3_PSK:
            return "WIFI_AUTH_WPA2_WPA3_PSK";
            break;
        default:
            return "WIFI_AUTH_UNKNOWN";
            break;
        }
    }

    const char *accessPoint::_pairwiseCipherAsString() const {
        switch (this->_record.pairwise_cipher) {
            case WIFI_CIPHER_TYPE_NONE:
                return "WIFI_CIPHER_TYPE_NONE";
                break;
            case WIFI_CIPHER_TYPE_WEP40:
                return "WIFI_CIPHER_TYPE_WEP40";
                break;
            case WIFI_CIPHER_TYPE_WEP104:
                return "WIFI_CIPHER_TYPE_WEP104";
                break;
            case WIFI_CIPHER_TYPE_TKIP:
                return "WIFI_CIPHER_TYPE_TKIP";
                break;
            case WIFI_CIPHER_TYPE_CCMP:
                return "WIFI_CIPHER_TYPE_CCMP";
                break;
            case WIFI_CIPHER_TYPE_TKIP_CCMP:
                return "WIFI_CIPHER_TYPE_TKIP_CCMP";
                break;
            default:
                return "WIFI_CIPHER_TYPE_UNKNOWN";
                break;
        }
    }

    const char *accessPoint::_groupCipherAsString() const {
        switch (this->_record.group_cipher) {
            case WIFI_CIPHER_TYPE_NONE:
                return "WIFI_CIPHER_TYPE_NONE";
                break;
            case WIFI_CIPHER_TYPE_WEP40:
                return "WIFI_CIPHER_TYPE_WEP40";
                break;
            case WIFI_CIPHER_TYPE_WEP104:
                return "WIFI_CIPHER_TYPE_WEP104";
                break;
            case WIFI_CIPHER_TYPE_TKIP:
                return "WIFI_CIPHER_TYPE_TKIP";
                break;
            case WIFI_CIPHER_TYPE_CCMP:
                return "WIFI_CIPHER_TYPE_CCMP";
                break;
            case WIFI_CIPHER_TYPE_TKIP_CCMP:
                return "WIFI_CIPHER_TYPE_TKIP_CCMP";
                break;
            default:
                return "WIFI_CIPHER_TYPE_UNKNOWN";
                break;
        }
    }

    std::ostream& operator<<(std::ostream& out,const accessPoint& ap) {
        out << "SSID\t\t" << ap._record.ssid << std::endl 
            << "BSSID\t\t" << std::hex << std::setfill('0') 
                            << std::setw(2) << std::right
                            << static_cast<int>(ap._record.bssid[0]) << ":" 
                            << std::setw(2) << std::right 
                            << static_cast<int>(ap._record.bssid[1]) << ":"
                            << std::setw(2) << std::right 
                            << static_cast<int>(ap._record.bssid[2]) << ":"
                            << std::setw(2) << std::right 
                            << static_cast<int>(ap._record.bssid[3]) << ":"
                            << std::setw(2) << std::right 
                            << static_cast<int>(ap._record.bssid[4]) << ":"
                            << std::setw(2) << std::right 
                            << static_cast<int>(ap._record.bssid[5])
                            << std::dec << std::endl
            << "RSSI\t\t" << static_cast<int16_t>(ap._record.rssi) << std::endl
            << "Authmode\t" <<  ap._authmodeAsString() << std::endl
            << "Pairw. Cipher\t" << ap._pairwiseCipherAsString() << std::endl
            << "Group Cipher\t" << ap._groupCipherAsString() << std::endl
            << "Channel\t\t" << static_cast<int16_t>(ap._record.primary) << std::endl
            << "Mode(s)\t\t" << (ap._record.phy_11b == 1 ? "802.11b " : "")
                            << (ap._record.phy_11g == 1 ? "802.11g " : "")
                            << (ap._record.phy_11n == 1 ? "802.11n " : "")
                            << (ap._record.phy_lr == 1 ? "802.11lr" : "") 
                            << std::endl
            << "Country\t\t" << ap._record.country.cc[0] 
                             << ap._record.country.cc[1] 
                             << ap._record.country.cc[2] << std::endl;
        return out;
    }

    std::ofstream& accessPoint::serialize(std::ofstream &os) const {
        os.write(reinterpret_cast<const char *>(&_record), sizeof(_record));
        os.write(reinterpret_cast<const char *>(&_lasterror), sizeof(_lasterror));
        os.write(reinterpret_cast<const char *>(&_valid), sizeof(_valid));
        return os;
    }

    std::ofstream& operator<<(std::ofstream& os,const accessPoint& ap) {
        switch(ap._record.ant) {
            case WIFI_ANT_ANT0:  
                os << (int)0;
                break;
            case WIFI_ANT_ANT1:
                os << (int)1;
                break;
            case WIFI_ANT_MAX:
                os << (int)2;
                break;
            default:
                os << (int)4;
        }
        os << ',';
        switch(ap._record.authmode) {
            case WIFI_AUTH_OPEN:
                os << (int)0;
                break;
            case WIFI_AUTH_WEP:
                os << (int)1;
                break;
            case WIFI_AUTH_WPA_PSK:
                os << (int)2;
                break;
            case WIFI_AUTH_WPA2_PSK:
                os << (int)3;
                break;
            case WIFI_AUTH_WPA_WPA2_PSK:
                os << (int)4;
                break;
            case WIFI_AUTH_WPA2_ENTERPRISE:
                os << (int)5;
                break;
            case WIFI_AUTH_WPA3_PSK:
                os << (int)6;
                break;
            case WIFI_AUTH_WPA2_WPA3_PSK:
                os << (int)7;
                break;
            case WIFI_AUTH_WAPI_PSK:
                os << (int)8;
                break;
            case WIFI_AUTH_MAX:
                os << (int)9;
                break;
            default:
                os << (int)10;  
        }
        os << ',';
        os << (int)ap._record.bssid[0];
        os << ',';
        os << ap._record.bssid[1];
        os << ',';
        os << ap._record.bssid[2];
        os << ',';
        os << ap._record.bssid[3];
        os << ',';
        os << ap._record.bssid[4];
        os << ',';
        os << ap._record.bssid[5];
        os << ',';
        os << ap._record.country.cc[0];
        os << ',';
        os << ap._record.country.cc[1];
        os << ',';
        os << ap._record.country.cc[2];
        os << ',';
        os << ap._record.country.max_tx_power;
        os << ',';
        os << ap._record.country.nchan;
        os << ',';
        switch(ap._record.country.policy) {
            case WIFI_COUNTRY_POLICY_AUTO:
                os << (int)0;
                break;
            case WIFI_COUNTRY_POLICY_MANUAL:
                os << (int)1;
                break;
            default:
                os << (int)2;
        }
        os << ',';
        os << ap._record.country.schan;
        os << ',';
        os << (int)ap._record.ftm_initiator;
        os << ',';
        os << (int)ap._record.ftm_responder;
        os << ',';
        switch(ap._record.group_cipher) {
            case WIFI_CIPHER_TYPE_NONE:
                os << (int)0;
                break;
            case WIFI_CIPHER_TYPE_WEP40:
                os << (int)1;
                break;
            case WIFI_CIPHER_TYPE_WEP104:
                os << (int)2;
                break;
            case WIFI_CIPHER_TYPE_TKIP:
                os << (int)3;
                break;
            case WIFI_CIPHER_TYPE_CCMP:
                os << (int)4;
                break;
            case WIFI_CIPHER_TYPE_TKIP_CCMP:
                os << (int)5;
                break;
            case WIFI_CIPHER_TYPE_AES_CMAC128:
                os << (int)6;
                break;
            case WIFI_CIPHER_TYPE_SMS4:
                os << (int)7;
                break;
            case WIFI_CIPHER_TYPE_GCMP:
                os << (int)8;
                break;
            case WIFI_CIPHER_TYPE_GCMP256:
                os << (int)9;
                break;
            case WIFI_CIPHER_TYPE_AES_GMAC128:
                os << (int)10;
                break;
            case WIFI_CIPHER_TYPE_AES_GMAC256:
                os << (int)11;
                break;
            case WIFI_CIPHER_TYPE_UNKNOWN:
                os << (int)12;
                break;
            default:
                os << (int)13;
        }
        os << ',';
        switch(ap._record.pairwise_cipher) {
            case WIFI_CIPHER_TYPE_NONE:
                os << (int)0;
                break;
            case WIFI_CIPHER_TYPE_WEP40:
                os << (int)1;
                break;
            case WIFI_CIPHER_TYPE_WEP104:
                os << (int)2;
                break;
            case WIFI_CIPHER_TYPE_TKIP:
                os << (int)3;
                break;
            case WIFI_CIPHER_TYPE_CCMP:
                os << (int)4;
                break;
            case WIFI_CIPHER_TYPE_TKIP_CCMP:
                os << (int)5;
                break;
            case WIFI_CIPHER_TYPE_AES_CMAC128:
                os << (int)6;
                break;
            case WIFI_CIPHER_TYPE_SMS4:
                os << (int)7;
                break;
            case WIFI_CIPHER_TYPE_GCMP:
                os << (int)8;
                break;
            case WIFI_CIPHER_TYPE_GCMP256:
                os << (int)9;
                break;
            case WIFI_CIPHER_TYPE_AES_GMAC128:
                os << (int)10;
                break;
            case WIFI_CIPHER_TYPE_AES_GMAC256:
                os << (int)11;
                break;
            case WIFI_CIPHER_TYPE_UNKNOWN:
                os << (int)12;
                break;
            default:
                os << (int)13;

        }
        os << ',';
        os << ap._record.phy_11b;
        os << ',';
        os << ap._record.phy_11g;
        os << ',';
        os << ap._record.phy_11n;
        os << ',';
        os << ap._record.phy_lr;
        os << ',';
        os << ap._record.primary;
        os << ',';
        os << ap._record.reserved;
        os << ',';
        os << ap._record.rssi;
        os << ',';
        switch(ap._record.second) {
            case  WIFI_SECOND_CHAN_NONE:
                os << 0;
                break;
            case WIFI_SECOND_CHAN_ABOVE:  
                os << 1;
                break;
            case WIFI_SECOND_CHAN_BELOW:  
                os << 2;
                break; 
            default:
                os << 3;
        }
        os << ',';
        for(int i = 0; i < 33; i++) {
            os << ap._record.ssid[i];
            os << ',';   
        }
        os << ap._record.wps;
        os << ',';
        os << ap._lasterror;
        os << ',';
        if(ap._valid) {
            os << (int)1;
        } else {
            os << (int)0;
        }
        return os;
    }

    std::ifstream& accessPoint::deserialize(std::ifstream &is) {
        is.read(reinterpret_cast<char *>(&_record), sizeof(_record));
        is.read(reinterpret_cast<char *>(&_lasterror), sizeof(_lasterror));
        is.read(reinterpret_cast<char *>(&_valid), sizeof(_valid));
        return is;
    }

    uint8_t *accessPoint::getSsid() {
        if(!_valid) {
            return NULL;
        }
        return _record.ssid;
    }

    wifi_err_t accessPoint::setSsid(const char &ssid) {
        // An SSID can have 32 characters and a null termination (33 in total). 
        // Check if the argument fulfills this requirement.
        size_t ssid_len = strlen(&ssid);
        if(ssid_len > SSID_MAX_LEN || ssid_len == 0) {
            ESP_LOGE(TAG_WIFI, "setSsid(): SSID not valid.");
            return WIFI_INVALID_ARG;
        }
        // Write the name to the record
        memcpy(_record.ssid, reinterpret_cast<const void*>(&ssid), ssid_len);

        // Pad the rest of the record with \0 to terminate string and avoid stale 
        // after terminating 0
        memset(reinterpret_cast<void *>(_record.ssid[ssid_len]), 0, (SSID_MAX_LEN+1) - ssid_len);  

        return WIFI_OK;
    }

    /*
     * Wifi Stations Config
     */

    /**
     * @brief Construct a new wifi Sta Config::wifi Sta Config object
     * 
     */
    wifiStaConfig::wifiStaConfig() {
        // Nothing to do, using defaults
    }

    /**
     * @brief Construct a new wifi Sta Config::wifi Sta Config object
     * 
     * Create the new object from a wifi_sta_config_t configuration.
     * 
     * @param cfg a wifi_sta_config_t configuration blueprint
     */
    wifiStaConfig::wifiStaConfig(const wifi_sta_config_t &cfg) {
        _config = cfg;
    }

    /**
     * @brief Assignment operator for wifiStaConfigs
     * 
     * @param rhs The right hand side wifiStaConfig
     * @return The left hand side of the assignment
     */
    wifiStaConfig &wifiStaConfig::operator=(const wifiStaConfig &rhs) {
        if(this == &rhs) {
            return *this;
        }

        this->_config = rhs._config;
        return *this;
    }

    /**
     * @brief Set the ssid of the configuration
     * 
     * Sets the ssid for this configuration. Ssids can be a maximum of 32 bytes.
     * If the provided ssid is less than 32 bytes, it must be 0 terminated. If 
     * the provided ssid is more than 32 bytes, it will be silently truncated.
     * 
     * TODO:
     *  - Currently we allow the empty string as ssid. Check if we should really
     *      do this.
     * 
     * @param ssid The ssid to set
     * @return wifi_err_t
     *          WIFI_OK on success, 
     *          WIFI_INVALID_ARG if argument is not valid
     */
    wifi_err_t wifiStaConfig::setSsid(const char *ssid) {
        
        // Check argument and abort if not valid
        if(ssid == NULL) {
            ESP_LOGE(TAG_WIFI, "setSsid(): received NULL pointer.");
            return WIFI_INVALID_ARG;
        }

        // Gather length information
        size_t ssid_len = 0;
        for(ssid_len = 0; ssid_len < SSID_MAX_LEN; ssid_len++) {
            if(ssid[ssid_len] == 0) {
                break;
            }
        }
       
        // Write the name to the record
        if(ssid_len != 0) {
            memcpy(_config.ssid, reinterpret_cast<const void*>(ssid), ssid_len);
        }

        // Pad the rest of the record with \0 to terminate string and avoid stale 
        // after terminating 0, unless the ssid is exactly SSID_MAX_LEN
        if(ssid_len < SSID_MAX_LEN) {
            memset(reinterpret_cast<void *>(&(_config.ssid[ssid_len])), 0, SSID_MAX_LEN - ssid_len);  
        }

        return WIFI_OK;
    }
    
    /**
     * @brief Retrieves the SSID in the config into a provided array
     * 
     * SSIDs have a maximum length of 32 bytes. Therefore the array provided to
     * the function must be at least 32 bytes long. If ssid is shorter than the
     * 32 bytes, it will be NULL terminated. If the ssid is exactly 32 bytes and
     * the array provided is longer, it will be null terminated. If the array
     * is exactly 32 bytes and the ssid is 32 bytes, there is no termination.
     * 
     * @param ssid An array of 'size' length (min 32 bytes)
     * @param size The size of the provided array
     * @return wifi_err_t 
     *          WIFI_OK if successful,
     *          WIFI_INALID_ARG if arguments not valid
     */
    wifi_err_t wifiStaConfig::getSsid(char *ssid, size_t size) {

        // Check that ssid array is large enough
        if(size < SSID_MAX_LEN || ssid == NULL) {
            ESP_LOGE(TAG_WIFI, "getSsid(): character buffer provided too small.");
            return WIFI_INVALID_ARG;
        }

        // Copy the content
        memcpy(ssid, reinterpret_cast<const char*>(_config.ssid), SSID_MAX_LEN);

        // If the size is larger than SSID_MAX_LEN, we ensure 0 termination 
        // even if the ssid is not due to it being SSID_MAX_LEN. Note: We just
        // set the 0 termination, even if there is potentially a previous one.
        // It does not hurt and is more efficient than checking.
        if(size > SSID_MAX_LEN) {
            ssid[SSID_MAX_LEN] = '\0';
        }
        return WIFI_OK;
    }
    
    /**
     * @brief Set the password
     * 
     * Sets the password for this configuration. Passwords can be a maximum of
     * 64 bytes. If the provided password is less than 64 bytes, it must be 0
     * terminated. If the provided password is more than 64 bytes, it will be
     * silently truncated.
     * 
     * @param pass Password, max 64 bytes. 0 terminated if less than 64 bytes.
     * @return wifi_err_t 
     *          WIFI_OK on success
     *          WIFI_INVALID_ARG if argument invalid
     */
    wifi_err_t wifiStaConfig::setPassword(const char *pass) {
        if(pass == NULL) {
            ESP_LOGE(TAG_WIFI, "setPassword(): received NULL pointer.");
            return WIFI_INVALID_ARG;
        } 

        // Calculate size
        size_t pass_len = 0;
        for(pass_len = 0; pass_len < PASS_MAX_LEN; pass_len++) {
            if(pass[pass_len] == 0) {
                break;
            }
        }        
       
        // Write the name to the record
        if(pass_len != 0) {
            memcpy(_config.password, reinterpret_cast<const void*>(pass), pass_len);
        }

        // Pad the rest of the record with \0 to terminate string and avoid stale 
        // after terminating 0, unless the ssid is exactly SSID_MAX_LEN
        if(pass_len < PASS_MAX_LEN) {
            memset(reinterpret_cast<void *>(&(_config.password[pass_len])), 0, PASS_MAX_LEN - pass_len);  
        }

        return WIFI_OK;
    }
            
    /**
     * @brief Retrieves the configured password into the provided array
     * 
     * Passwords have a maximum length of 64 bytes. Therefore the array provided 
     * to the function must be at least 64 bytes long. If password is shorter 
     * than the 64 bytes, it will be 0 terminated. If the password is exactly 64 
     * bytes and the array provided is longer, it will be 0 terminated. If the 
     * array is exactly 64 bytes and the password is 64 bytes, there is no 
     * termination.
     * 
     * TODO:
     *  - We could check the actual size of the password and accept the pass
     *      array if it is smaller than PASS_MAX_LEN. This would come at the
     *      cost of more computation time.
     * 
     * @param pass   The array to store the password (min 64 bytes)
     * @param length The length of the pass array
     * @return wifi_err_t WIFI_OK if successfull,
     *          WIFI_INVALID_ARG if arguments are invalid
     */
    wifi_err_t wifiStaConfig::getPassword(char *pass, size_t length) {
        // Check arguments
        if(length < PASS_MAX_LEN || pass == NULL) {
            ESP_LOGE(TAG_WIFI, "getPassword(): character buffer provided too small.");
            return WIFI_INVALID_ARG;
        }

        // Copy all of the password, including potential garbage after 0
        memcpy(pass, reinterpret_cast<const char*>(_config.password), PASS_MAX_LEN);

        // If pass array longer than PASS_MAX_LEN, add terminating 0. This can
        // be done even if there is 0 termination before. (more efficient than
        // checking)
        if(length > PASS_MAX_LEN) {
            pass[PASS_MAX_LEN] = '\0';
        }
        return WIFI_OK;
    }
    
    /**
     * @brief Set the scan method
     * 
     * The scan method can either be WIFI_ALL_CHANNEL_SCAN or WIFI_FAST_SCAN.
     * If fast scan is set thresholds will be used.
     * 
     * @param method The scan method to use. (WIFI_ALL_CHANNEL_SCAN or WIFI_FAST_SCAN).
     */
    void wifiStaConfig::setScanMethod(wifi_scan_method_t method) {
        _config.scan_method = method;
    }
    
    /**
     * @brief Get the scan method
     * 
     * @return wifi_scan_method_t WIFI_FAST_SCAN or WIFI_ALL_CHANNEL_SCAN
     */
    wifi_scan_method_t wifiStaConfig::getScanMethod() {
        return _config.scan_method;
    }

    /**
     * @brief set the BSSID of the access point to connect to
     * 
     * This allows restricting the access point to connect to to a single BSSID.
     * 
     * Note: In order to make this work you need also to enforce the BSSID by
     *          setting setEnforceBSSID(true).
     * 
     * @param bssid the BSSID of the access point
     * @return wifi_err_t 
     *          WIFI_OK on success.
     */
    wifi_err_t wifiStaConfig::setBssid(uint8_t (&bssid)[6]) {
        memcpy(reinterpret_cast<char *>(_config.bssid), reinterpret_cast<const char*>(&bssid), sizeof(_config.bssid));
        return WIFI_OK;
    }
    
    wifi_err_t wifiStaConfig::getBssid(uint8_t (&bssid)[6]) {
        memcpy(reinterpret_cast<char *>(&bssid), reinterpret_cast<const char*>(_config.bssid), sizeof(bssid));
        return WIFI_OK;
    }
            
    void wifiStaConfig::setEnforceBssid(bool enforce) {
        _config.bssid_set = enforce;
    }

    bool wifiStaConfig::getEnforceBssid() {
        return _config.bssid_set;
    }
    
    wifi_err_t wifiStaConfig::setChannel(uint8_t channel){
        if(channel > 13) {
            ESP_LOGE(TAG_WIFI, "Channel out of range.");
            return WIFI_INVALID_ARG;
        }
        _config.channel = channel;
        return WIFI_OK;
    }
    
    uint8_t wifiStaConfig::getChannel(){
        return _config.channel;
    }
    
    void wifiStaConfig::setListenInterval(uint16_t interval) {
        _config.listen_interval = interval;
    }
    
    uint16_t wifiStaConfig::getListenInterval() {
        return _config.listen_interval;
    }

    wifi_err_t wifiStaConfig::setSortMethod(wifi_sort_method_t method) {
        return _config.sort_method = method;
    }
            
    wifi_sort_method_t wifiStaConfig::getSortMethod() {
        return _config.sort_method;
    }
    
    void wifiStaConfig::setScanThreshold(wifi_scan_threshold_t threshold) {
        _config.threshold = threshold;
    }
    
    wifi_scan_threshold_t wifiStaConfig::getScanThreshold() {
        return _config.threshold;
    }

    void wifiStaConfig::setScanThresholdRSSI(uint8_t rssi) {
        _config.threshold.rssi = rssi;
    }

    uint8_t wifiStaConfig::getScanThresholdRSSI() {
        return _config.threshold.rssi;
    }

    void wifiStaConfig::setScanThresholdAuthMode(wifi_auth_mode_t mode) {
        _config.threshold.authmode = mode;
    }

    wifi_auth_mode_t wifiStaConfig::getScanThresholdAuthMode() {
        return _config.threshold.authmode;
    }

    /**
     * @brief Get the auth mode as string
     * 
     * Will fetch the authmode into str. If the length of the provided character
     * array is not long enough, it will be silently truncated.
     * 
     * Note: The longest possible value is 26 characters (without terminating 0)
     * 
     * @param str 
     * @param len 
     */
    void wifiStaConfig::getScanThresholdAuthModeAsString(char *str, size_t len) {
        return _authmodeToString(_config.threshold.authmode, str, len);
    }

    void wifiStaConfig::setPMFConfig(wifi_pmf_config_t cfg) {
        _config.pmf_cfg = cfg;
    }

    wifi_pmf_config_t wifiStaConfig::getPMFConfig() {
        return _config.pmf_cfg;
    }

    void wifiStaConfig::setPMFCapable(bool capable) {
        _config.pmf_cfg.capable = capable;
    }

    bool wifiStaConfig::getPMFCapable() {
        return _config.pmf_cfg.capable;
    }

    void wifiStaConfig::setPMFRequired(bool required) {
        _config.pmf_cfg.required = required;
    }

    bool wifiStaConfig::getPMFRequired() {
        return _config.pmf_cfg.required;
    }

    void wifiStaConfig::setRM(bool rm_enabled) {
        _config.rm_enabled = rm_enabled;
    }

    bool wifiStaConfig::getRM() {
        return _config.rm_enabled;
    }

    void wifiStaConfig::setBTM(bool btm_enabled) {
        _config.btm_enabled = btm_enabled;
    }

    bool wifiStaConfig::getBTM() {
        return _config.btm_enabled;
    }

    void wifiStaConfig::setMBO(bool mbo_enabled) {
        _config.mbo_enabled = mbo_enabled;
    }

    bool wifiStaConfig::getMBO() {
        return _config.mbo_enabled;
    }

    void wifiStaConfig::setFTEnabled(bool ft_enabled) {
        _config.ft_enabled = ft_enabled;
    }

    bool wifiStaConfig::getFTEnabled() {
        return _config.ft_enabled;
    }

    void wifiStaConfig::setOWEEnabled(bool owe_enabled) {
        _config.owe_enabled = owe_enabled;
    }

    bool wifiStaConfig::getOWEEnabled() {
        return _config.owe_enabled;
    }

    void wifiStaConfig::setTransitionDisable(bool transition_disable) {
        _config.transition_disable = transition_disable;
    }

    bool wifiStaConfig::getTransitionDisable() {
        return _config.transition_disable;
    }

    void wifiStaConfig::setSaePweH2e(wifi_sae_pwe_method_t sae_pwe_h2e) {
        _config.sae_pwe_h2e = sae_pwe_h2e;
    }

    wifi_sae_pwe_method_t wifiStaConfig::getSaePweH2e() {
        return _config.sae_pwe_h2e;
    }

    void wifiStaConfig::setFailureRetryCount(uint8_t failure_retry_count) {
        _config.failure_retry_cnt = failure_retry_count;
    }

    uint8_t wifiStaConfig::getFailureRetryCount() {
        return _config.failure_retry_cnt;
    }

    std::ofstream& wifiStaConfig::serialize(std::ofstream &os) const {
        os.write(reinterpret_cast<const char *>(&_version), sizeof(_version));
        os.write(reinterpret_cast<const char *>(&_config), sizeof(_config));
        return os;
    }

    // TODO - maybe we should check for read errors 
    std::ifstream& wifiStaConfig::deserialize(std::ifstream &is) {
        uint8_t version;
        is.read(reinterpret_cast<char *>(&version), sizeof(version));
        if(version != _version) {
            ESP_LOGE(TAG_WIFI, "deserialize(): Version mismatch. Aborting.");
            is.putback(version);
            is.setstate(std::ios_base::failbit);
            return is;
        }
        is.read(reinterpret_cast<char *>(&_config), sizeof(_config));
        return is;
    }

    void wifiStaConfig::_authmodeToString(wifi_auth_mode_t mode, char *str, size_t len) {
        switch(mode) {
            case WIFI_AUTH_OPEN:
                strncpy(str, "WIFI_AUTH_OPEN", len);
                break;
            case WIFI_AUTH_WEP:
                strncpy(str, "WIFI_AUTH_WEP", len);
                break;
            case WIFI_AUTH_WPA_PSK:
                strncpy(str, "WIFI_AUTH_WPA_PSK", len);
                break;
            case WIFI_AUTH_WPA2_PSK:
                strncpy(str, "WIFI_AUTH_WPA2_PSK", len);
                break;
            case WIFI_AUTH_WPA_WPA2_PSK:
                strncpy(str, "WIFI_AUTH_WPA_WPA2_PSK", len);
                break;
            case WIFI_AUTH_WPA2_ENTERPRISE:
                strncpy(str, "WIFI_AUTH_WPA2_ENTERPRISE", len);
                break;
            case WIFI_AUTH_WPA3_PSK:
                strncpy(str, "WIFI_AUTH_WPA3_PSK", len);
                break;
            case WIFI_AUTH_WPA2_WPA3_PSK:
                strncpy(str, "WIFI_AUTH_WPA2_WPA3_PSK", len);
                break;
            case WIFI_AUTH_WAPI_PSK:
                strncpy(str, "WIFI_AUTH_WAPI_PSK", len);
                break;
            default:
                strncpy(str, "WIFI_AUTH_UNKNOWN", len);
                break;
        }
    }

    

    /*
     * wifiController
     */


    // Private memeber initialisation
    accessPointList wifiController::_ap_list{};
    char wifiController::_mac_addr_cstr[]{};
    std::mutex wifiController::_mutx{};
    wifiController::state_e wifiController::_state{state_e::NOT_INITIALIZED};
    wifi_init_config_t wifiController::_wifi_init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    wifi_config_t wifiController::_wifi_cfg{};

    /**
     * @brief Construct a new wifi Controller::wifi Controller object
     * 
     */
    wifiController::wifiController(void) {
        // Set the log level for the WiFi as defined in menuconfig
        esp_log_level_set(TAG_WIFI, WIFI_LOG_LEVEL);
        esp_log_level_set(TAG_WIFI_DRIVER, WIFI_DRIVER_LOG_LEVEL);
        //esp_log_level_set("proto_wifi_scan", WIFI_LOG_LEVEL);
        // wifi_prov_scheme_console
        // wifi_prov_scheme_softap
        // wifi_prov_mgr
        // wifi_prov_handlers
        // wifi dpp-enrollee

        // Get own mac address                                
        if (!_mac_addr_cstr[0]) {
            if (ESP_OK != _get_mac()) {
                ESP_LOGE(TAG_WIFI, "Could not get mac address. WiFi not working.");
                esp_restart(); // TODO exchange with proper error handling
            }
        }

        // Initialise default config
        // TODO make configurable through menuconfig
        memset(_wifi_cfg.sta.ssid, 0, sizeof(_wifi_cfg.sta.ssid));
        memset(_wifi_cfg.sta.password, 0, sizeof(_wifi_cfg.sta.password));
        _wifi_cfg.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
        _wifi_cfg.sta.bssid_set = false;
        memset(_wifi_cfg.sta.bssid, 0, sizeof(_wifi_cfg.sta.bssid));
        _wifi_cfg.sta.channel = 0;
        _wifi_cfg.sta.listen_interval = 0;
        _wifi_cfg.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
        _wifi_cfg.sta.threshold = {
            .rssi = 0,
            .authmode = WIFI_AUTH_OPEN,
        };
        _wifi_cfg.sta.pmf_cfg = {
            .capable = true,
            .required = false,
        };
        _wifi_cfg.sta.rm_enabled = true;
        _wifi_cfg.sta.btm_enabled = true;
        _wifi_cfg.sta.mbo_enabled = true;
        _wifi_cfg.sta.ft_enabled = true;
        _wifi_cfg.sta.owe_enabled = true;
        _wifi_cfg.sta.transition_disable = true;
        _wifi_cfg.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;
        _wifi_cfg.sta.failure_retry_cnt = 3;

    }

    wifiController::wifiController(wifiStaConfig &cfg) {
        setStaConfig(cfg);
    }

    void wifiController::setStaConfig(wifiStaConfig &cfg) {
        cfg.getSsid(reinterpret_cast<char*>(_wifi_cfg.sta.ssid), sizeof(_wifi_cfg.sta.ssid));
        cfg.getPassword(reinterpret_cast<char*>(_wifi_cfg.sta.password), sizeof(_wifi_cfg.sta.password));
        _wifi_cfg.sta.scan_method = cfg.getScanMethod();
        _wifi_cfg.sta.bssid_set = cfg.getEnforceBssid();
        cfg.getBssid(_wifi_cfg.sta.bssid);
        _wifi_cfg.sta.channel = cfg.getChannel();
        _wifi_cfg.sta.listen_interval = cfg.getListenInterval();
        _wifi_cfg.sta.sort_method = cfg.getSortMethod();
        _wifi_cfg.sta.threshold = cfg.getScanThreshold();
        _wifi_cfg.sta.pmf_cfg = cfg.getPMFConfig();
        _wifi_cfg.sta.rm_enabled = cfg.getRM();
        _wifi_cfg.sta.btm_enabled = cfg.getBTM();
        _wifi_cfg.sta.mbo_enabled = cfg.getMBO();
        _wifi_cfg.sta.ft_enabled = cfg.getFTEnabled();
        _wifi_cfg.sta.sae_pwe_h2e = cfg.getSaePweH2e();
        _wifi_cfg.sta.failure_retry_cnt = cfg.getFailureRetryCount();
    }

    wifiStaConfig wifiController::getStaConfig() { 
        return wifiStaConfig(_wifi_cfg.sta);
    }

    // Event handler
    void wifiController::wifi_event_handler(void *arg, esp_event_base_t event_base,
                                  int32_t event_id, void *event_data) {
        ESP_LOGV(TAG_WIFI, "Event handler called.");
        if (WIFI_EVENT == event_base) {
            const wifi_event_t event_type{static_cast<wifi_event_t>(event_id)};

            /*

                Events not yet handled

                WIFI_EVENT_STA_WPS_ER_SUCCESS,       ESP32 station wps succeeds in enrollee mode 
                WIFI_EVENT_STA_WPS_ER_FAILED,        ESP32 station wps fails in enrollee mode 
                WIFI_EVENT_STA_WPS_ER_TIMEOUT,       ESP32 station wps timeout in enrollee mode 
                WIFI_EVENT_STA_WPS_ER_PIN,           ESP32 station wps pin code in enrollee mode 
                WIFI_EVENT_STA_WPS_ER_PBC_OVERLAP,   ESP32 station wps overlap in enrollee mode 

                WIFI_EVENT_AP_START,                 ESP32 soft-AP start 
                WIFI_EVENT_AP_STOP,                  ESP32 soft-AP stop 
                WIFI_EVENT_AP_STACONNECTED,          a station connected to ESP32 soft-AP 
                WIFI_EVENT_AP_STADISCONNECTED,       a station disconnected from ESP32 soft-AP 
                WIFI_EVENT_AP_PROBEREQRECVED,        Receive probe request packet in soft-AP interface 

                WIFI_EVENT_FTM_REPORT,               Receive report of FTM procedure 

                Add next events after this only 
                WIFI_EVENT_STA_BSS_RSSI_LOW,         AP's RSSI crossed configured threshold 
                WIFI_EVENT_ACTION_TX_STATUS,         Status indication of Action Tx operation 
                WIFI_EVENT_ROC_DONE,                 Remain-on-Channel operation complete 

                WIFI_EVENT_STA_BEACON_TIMEOUT,       ESP32 station beacon timeout 

                WIFI_EVENT_MAX,                      Invalid WiFi event ID 
            */
            switch (event_type) {
                case WIFI_EVENT_STA_START:
                {
                    ESP_LOGV(TAG_WIFI, "ESP32 station started.");

                    std::lock_guard<std::mutex> state_guard(_mutx);
                    _state = state_e::READY_TO_CONNECT;
                    break;
                }

                case WIFI_EVENT_STA_STOP:
                {
                    ESP_LOGW(TAG_WIFI, "STA STOP event handling not yet implemented.\n");

                    //std::lock_guard<std::mutex> state_guard(_mutx);
                    // TODO _state = state_e::READY_TO_CONNECT;
                    break;
                }

                case WIFI_EVENT_WIFI_READY:
                {
                    ESP_LOGW(TAG_WIFI, "WIFI READY event handling not yet implemented.\n");

                    //std::lock_guard<std::mutex> state_guard(_mutx);
                    // TODO _state = state_e::READY_TO_CONNECT;
                    break;
                }

                case WIFI_EVENT_SCAN_DONE:
                {
                    ESP_LOGW(TAG_WIFI, "SCAN DONE event handling not yet implemented.\n");

                    //std::lock_guard<std::mutex> state_guard(_mutx);
                    // TODO _state = state_e::READY_TO_CONNECT;
                    break;
                }

                case WIFI_EVENT_STA_CONNECTED:
                {
                    ESP_LOGI(TAG_WIFI, "Connected to accesspoint.");

                    std::lock_guard<std::mutex> state_guard(_mutx);
                    _state = state_e::WAITING_FOR_IP;
                    break;
                }

                case WIFI_EVENT_STA_DISCONNECTED:
                {
                    ESP_LOGI(TAG_WIFI, "Disconnected from accesspoint.");

                    std::lock_guard<std::mutex> state_guard(_mutx);
                    _state = state_e::DISCONNECTED;
                    break;
                }

                case WIFI_EVENT_STA_AUTHMODE_CHANGE:
                {
                    ESP_LOGW(TAG_WIFI, "AUTHMODE CHANGE event not yet implementd.");
                    break;
                }

                default:
                    ESP_LOGW(TAG_WIFI,"Got a unknown event");
                    break;
            }
        }
    }

    // IP Event handler
    void wifiController::ip_event_handler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data) {
        if (IP_EVENT == event_base) {
            const ip_event_t event_type{static_cast<ip_event_t>(event_id)};

            switch (event_type) {
                case IP_EVENT_STA_GOT_IP:
                {
                    std::lock_guard<std::mutex> state_guard(_mutx);
                    _state = state_e::CONNECTED;
                    break;
                }

                case IP_EVENT_STA_LOST_IP:
                {
                    std::lock_guard<std::mutex> state_guard(_mutx);
                    if (state_e::DISCONNECTED != _state)
                    {
                        _state = state_e::WAITING_FOR_IP;
                    }
                    break;
                }

                default:
                    break;
            }
        }
    }

    /**
     * @brief Connect with the given configuration
     * 
     * Note: This will connect either as station or operate in access point mode
     *          depending on the set configuration.
     * 
     * TODO:
     *  - Implement blocking mode
     *  - Access point mode currently unsupported - implement
     * 
     * @return esp_err_t 
     */
    esp_err_t wifiController::connect(bool blocking) {
        std::lock_guard<std::mutex> connect_guard(_mutx);
        esp_err_t status{ESP_OK};

        ESP_LOGD(TAG_WIFI, "Connection wifi...");

        switch (_state) {
            case state_e::READY_TO_CONNECT:
                [[fallthrough]];
            case state_e::DISCONNECTED:
                // Design decision: We set the configuration before attempting
                // to connect rather than requiring a manual setting.
                // To ensure object state, we take a snapshot of the current
                // active configuration and restore it if needed.
                wifi_config_t tempcfg;
                status = esp_wifi_get_config(WIFI_IF_STA, &tempcfg);
                if(status != ESP_OK) {
                    ESP_LOGE(TAG_WIFI, "Could not save current config.");
                    break;
                }

                // TODO - we might not need this or we do toggle to test
                /*
                if((status = esp_wifi_stop()) != ESP_OK) {
                    ESP_LOGE(TAG_WIFI, "Could not stop wifi.");
                    break;
                }
                */

                status = esp_wifi_set_config(WIFI_IF_STA, &_wifi_cfg);
                if(status == ESP_OK) {
                    /*
                    status = esp_wifi_start();
                    if(status != ESP_OK) {
                        ESP_LOGE(TAG_WIFI, "Could not start wifi.");
                        break;
                    }
                    */
                    status = esp_wifi_connect();
                    ESP_LOGD(TAG_WIFI, "Ready to connect to %s. Status is %d", _wifi_cfg.sta.ssid, status);
                    if (status == ESP_OK) {
                        _state = state_e::CONNECTING;
                    }
                } else {
                    // Try to restore the configuration and restart the ap
                    // No need to check success, nothing much we can do anyways.
                    ESP_LOGE(TAG_WIFI, "Failed to set new configuration Trying to revert...");
                    esp_wifi_set_config(WIFI_IF_STA, &tempcfg);
                    esp_wifi_start();
                }
                break;
            case state_e::CONNECTING:
                [[fallthrough]];
            case state_e::WAITING_FOR_IP:
                [[fallthrough]];
            case state_e::CONNECTED:
                ESP_LOGD(TAG_WIFI, "Already connected. Disconnect first.");
                status = WIFI_CONN_ERR;
                break;
            case state_e::NOT_INITIALIZED:
                ESP_LOGD(TAG_WIFI, "Wifi not initialised. Cannot connect.");
                status = WIFI_NOT_INIT;
                break;
            case state_e::INITIALIZED:
                ESP_LOGD(TAG_WIFI, "Initialised, but not started? Cannot connect.");
                status = WIFI_NOT_STARTED;
                break;
            case state_e::ERROR:
                ESP_LOGD(TAG_WIFI, "Unknown error. Cannot connect.");
                status = ESP_FAIL;
                break;
        }
        return status;
    }

    /**
     * @brief 
     * 
     * TODO:
     *  - Implement blocking mode
     * 
     * @param blocking 
     * @return esp_err_t 
     */
    esp_err_t wifiController::disconnect(bool blocking) {
        std::lock_guard<std::mutex> connect_guard(_mutx);
        esp_err_t status{ESP_OK};

        ESP_LOGD(TAG_WIFI, "Connection wifi with current configuration....");

        switch (_state) {
            case state_e::READY_TO_CONNECT:
                [[fallthrough]];
            case state_e::DISCONNECTED:
                ESP_LOGW(TAG_WIFI, "disconnect(): Already disconnected. Doing nothing.");
                break;
            case state_e::CONNECTING:
                [[fallthrough]];
            case state_e::WAITING_FOR_IP:
                [[fallthrough]];
            case state_e::CONNECTED:
                ESP_LOGD(TAG_WIFI, "disconnect(): Disconnecting......");
                status = esp_wifi_disconnect();
                break;
            case state_e::NOT_INITIALIZED:
                ESP_LOGD(TAG_WIFI, "disconnect(): Not initialised. Cannot disconnect.");
                status = ESP_ERR_WIFI_NOT_INIT;
                break;
            case state_e::INITIALIZED:
                ESP_LOGD(TAG_WIFI, "disconnect(): Initialised, but not started? Cannot disconnect.");
                status = ESP_ERR_WIFI_NOT_STARTED; // Verify this is correct
                break;
            case state_e::ERROR:
                ESP_LOGD(TAG_WIFI, "disconnect(): Unknown error.");
                status = ESP_FAIL;
                break;
        }
        return status;
    }

    wifi_err_t wifiController::_init() {
        std::lock_guard<std::mutex> mutx_guard(_mutx);

        esp_err_t status{ESP_OK};

        if (_state == state_e::NOT_INITIALIZED) {
            status |= esp_netif_init();
            if (status == ESP_OK) {
                const esp_netif_t *const p_netif = esp_netif_create_default_wifi_sta();

                if (!p_netif) {
                    status = ESP_FAIL;
                }
            }

            if (ESP_OK == status) {
                status = esp_wifi_init(&_wifi_init_cfg);
            }

            if (ESP_OK == status) {
                status = esp_event_handler_instance_register(WIFI_EVENT,
                                                             ESP_EVENT_ANY_ID,
                                                             &wifi_event_handler,
                                                             nullptr,
                                                             nullptr);
            }

            if (ESP_OK == status) {
                status = esp_event_handler_instance_register(IP_EVENT,
                                                             ESP_EVENT_ANY_ID,
                                                             &ip_event_handler,
                                                             nullptr,
                                                             nullptr);
            }

            // TODO implement support for WIFI_MODE_AP
            if (ESP_OK == status) {
                status = esp_wifi_set_mode(WIFI_MODE_STA);
            }

            // TODO check if we actually need to start wifi here or if we can
            // do later
            if (ESP_OK == status) {
                // _wifi_cfg is initialised in constructor or by user
                status = esp_wifi_set_config(WIFI_IF_STA, &_wifi_cfg);
            }

            if (ESP_OK == status) {
                status = esp_wifi_start(); // start Wifi
            }

            if (ESP_OK == status) {
                _state = state_e::INITIALIZED;
            }
        } else if (_state == state_e::ERROR) {
            ESP_LOGE(TAG_WIFI, "WiFi is in error state. Resetting to not initialised.");
            _state = state_e::NOT_INITIALIZED;
        } else {
            ESP_LOGW(TAG_WIFI, "WiFi already initialised. Skipping initialisation.");
        }

        return translate_esp_err_t(status);
    }

    wifiController::state_e wifiController::getWifiState() {
        std::lock_guard<std::mutex> state_guard(_mutx);  
        return _state;
    }

    void wifiController::setSsid(const char *ssid) {
        // TODO fix - currently does not proper 0 terminate
        memcpy(_wifi_cfg.sta.ssid, ssid, std::min(strlen(ssid)+1, sizeof(_wifi_cfg.sta.ssid)));
    }

    void wifiController::setPassword(const char *password) {
        // TODO fix - currently does not properly 0 terminate
        memcpy(_wifi_cfg.sta.password, password, std::min(strlen(password)+1, sizeof(_wifi_cfg.sta.password)));
    }

    wifi_err_t wifiController::init() {
        return translate_esp_err_t(_init());      
    }

    // Get default MAC from API and convert to ASCII HEX
    wifi_err_t wifiController::_get_mac(void) {
        uint8_t mac_byte_buffer[6]{};

        const esp_err_t status{esp_efuse_mac_get_default(mac_byte_buffer)};

        if (status == ESP_OK) {
            snprintf(_mac_addr_cstr, sizeof(_mac_addr_cstr), "%02X%02X%02X%02X%02X%02X",
                     mac_byte_buffer[0],
                     mac_byte_buffer[1],
                     mac_byte_buffer[2],
                     mac_byte_buffer[3],
                     mac_byte_buffer[4],
                     mac_byte_buffer[5]);
            return WIFI_OK;
        } else {
            return WIFI_INVALID_ARG;
        }
    }

    wifi_err_t wifiController::scanAPs() {
        // TODO - ensure wifi is started and set to AP mode
        // ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        // ESP_ERROR_CHECK(esp_wifi_start());
        esp_err_t err{ESP_OK};
        uint16_t number = WIFI_SCAN_LIST_SIZE;
        wifi_ap_record_t ap_info[WIFI_SCAN_LIST_SIZE];
        uint16_t ap_count = 0;
        
        // Initialise ap_info and ap_list
        memset(ap_info, 0, sizeof(ap_info));
        _ap_list.clear();

        err = esp_wifi_scan_start(NULL, true);

        if(err == ESP_OK) {
            err = esp_wifi_scan_get_ap_records(&number, ap_info);
            if(err == ESP_OK) {
                err = esp_wifi_scan_get_ap_num(&ap_count);
                if(err == ESP_OK) {
                    for (int i = 0; (i <  WIFI_SCAN_LIST_SIZE) && (i < ap_count); i++) {
                        _ap_list.addAccessPoint(accessPoint(ap_info[i]));
                    }
                }
            }
        } 
        return err;
    }

    accessPointList wifiController::getAPList() {
        return _ap_list;
    }

    wifi_err_t wifiController::staGetAPInfo(accessPoint &ap) {
        wifi_ap_record_t ap_record;
        esp_err_t status = esp_wifi_sta_get_ap_info(&ap_record);
        if(status == ESP_OK) {
            ap = accessPoint(ap_record);
            return WIFI_OK;
        } else {
            switch(status) {
                case ESP_ERR_WIFI_CONN:
                    return WIFI_NOT_INIT;
                    break;

                case ESP_ERR_WIFI_NOT_CONNECT:
                    return WIFI_NOT_CONNECTED;
                    break;

                default:
                    return WIFI_FAIL;
            }
        }
    }

    // TODO change to make this taking an accessPoint object and persist it.
    wifi_err_t wifiController::saveAP(char *ap_ssid, char *ap_pass) {

        // Check that we received a valid access point ssid
        // TODO - detect disallowed charaters --> need to read the standard
        if(ap_ssid == NULL) {
            ESP_LOGE(TAG_WIFI, "saveAP: Invalid access point ssid.");
            return WIFI_INVALID_ARG;
        } 
        
        // Construct the filename
        char *fname = (char *)malloc(strlen("/spiffs/") + strlen(WIFI_AP_SAVE_FILE) +1);
        if(fname == NULL) {
            ESP_LOGE(TAG_WIFI, "saveAP: Could not allocate memory for filename.");
            return WIFI_NO_MEM;
        }
        strcpy(fname, "/spiffs/");
        strcat(fname, WIFI_AP_SAVE_FILE);
        ESP_LOGI(TAG_WIFI, "saveAP: Storing in: %s", fname);

        // Open the file and append the access point information
        std::ofstream file(fname, std::ios::app);
        if(file.is_open()) {
            file << ap_ssid << ',';
            if(ap_pass != NULL) {
                file << ap_pass;
            }
            file << std::endl;
            file.close();
        } else {
            ESP_LOGE(TAG_WIFI, "saveAP: Could not open file '%s' to store ap information", "/spiffs/test");
            free(fname);
            return WIFI_PERSIST_FAIL;
        }


        // DEBUG BEGIN
        // print the content
        // TODO remove
        FILE *f = fopen(fname, "r");
        if(f != NULL) {
            char line[256];
            while (fgets(line, sizeof(line), f) != NULL) {
                printf("Line found: ");
                printf(line);
            }
            printf("Done\n");
            fclose(f);
        } else {
            printf("Could not open file for reading.\n");
        }
        // DEBUG END

        free(fname);
        return WIFI_OK;
    }

// DEBUG START
void hexdump(void *ptr, int buflen)
{
   unsigned char *buf = (unsigned char*)ptr;
   int i, j;
   for (i=0; i<buflen; i+=16) {
      printf("%06x: ", i);
      for (j=0; j<16; j++) { 
         if (i+j < buflen)
            printf("%02x ", buf[i+j]);
         else
            printf("   ");
      }
      printf(" ");
      for (j=0; j<16; j++) {
         if (i+j < buflen)
            printf("%c", isprint(buf[i+j]) ? buf[i+j] : '.');
      }
      printf("\n");
   }
}
// DEBUG END

    // TODO change to make this taking an accessPoint object and persist it.
    wifi_err_t wifiController::saveConfig(const wifiStaConfig &cfg) {

        // Check that we received a valid access point ssid
        // TODO - detect disallowed charaters --> need to read the standard
        // TODO - error checking of bject.
        
        // Construct the filename
        char *fname = (char *)malloc(strlen("/spiffs/") + strlen(WIFI_AP_SAVE_FILE) +1);
        if(fname == NULL) {
            ESP_LOGE(TAG_WIFI, "saveAP: Could not allocate memory for filename.");
            return WIFI_NO_MEM;
        }
        strcpy(fname, "/spiffs/");
        strcat(fname, WIFI_AP_SAVE_FILE);
        ESP_LOGI(TAG_WIFI, "saveAP: Storing in: %s", fname);

        // Open the file and append the access point information
        std::ofstream file(fname, std::ios::app | std::ios::binary);
        if(file.is_open()) {
            //file << ap;
            cfg.serialize(file); 
            file.close();
        } else {
            ESP_LOGE(TAG_WIFI, "saveAP: Could not open file '%s' to store ap information", fname);
            free(fname);
            return WIFI_PERSIST_FAIL;
        }


        // DEBUG BEGIN
        std::ifstream infile(fname, std::ios::in | std::ios::binary);
        if(infile.is_open()) {
            // get the starting position
            std::streampos start = infile.tellg();

            // go to the end
            infile.seekg(0, std::ios::end);

            // get the ending position
            std::streampos end = infile.tellg();

            // go back to the start
            infile.seekg(0, std::ios::beg);

            // create a vector to hold the data that
            // is resized to the total size of the file    
            std::vector<char> contents;
            contents.resize(static_cast<size_t>(end - start));

            // read it in
            infile.read(&contents[0], contents.size());

            // print it out (for clarity)
            hexdump(contents.data(), contents.size());
        } else {
            ESP_LOGE(TAG_WIFI, "saveAP: Could not open file '%s' to read ap information", fname);
            free(fname);
            return WIFI_FILE_OPEN_FAIL;
        }
        // DEBUG END

        infile.close();
        free(fname);
        return WIFI_OK;
    }

    wifi_err_t wifiController::loadConfigs(std::vector<wifiStaConfig> &cfg_list) {
        // Construct the filename
        char *fname = (char *)malloc(strlen("/spiffs/") + strlen(WIFI_AP_SAVE_FILE) +1);
        if(fname == NULL) {
            ESP_LOGE(TAG_WIFI, "loadConfigs: Could not allocate memory for filename.");
            return WIFI_NO_MEM;
        }
        strcpy(fname, "/spiffs/");
        strcat(fname, WIFI_AP_SAVE_FILE);
        ESP_LOGI(TAG_WIFI, "loadConfigs: Loading from: %s", fname);

        // Open file for reading
        std::ifstream infile(fname, std::ios::in | std::ios::binary);
        if(infile.is_open()) {
            wifiStaConfig cfg;

            // We need to look ahead as otherwise eof is reached in deserialize
            // attempt.
            int lookahead = infile.peek();
            while(infile.good() && lookahead != EOF) {
                cfg.deserialize(infile);
                if(infile.good()) {
                    cfg_list.push_back(cfg);
                    lookahead = infile.peek();
                }
            } 
            // TODO if we have not reached EOF, then something went wrong
            if(lookahead != EOF || !infile.eof()) {
                ESP_LOGE(TAG_WIFI, "Could not read all configurations, file corrupted.");
            }
        } else {
            ESP_LOGE(TAG_WIFI, "loadConfigs: Could not open file '%s' to read configurations", fname);
            free(fname);
            return WIFI_FILE_OPEN_FAIL;
        }

        free(fname);
        infile.close();
        return WIFI_OK;
    }
} // namespace espwifi