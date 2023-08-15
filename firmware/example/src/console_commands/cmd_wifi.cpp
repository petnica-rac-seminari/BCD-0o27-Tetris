/**
 * @file cmd_wifi.c
 * @author Florian Schuetz (fschuetz@ieee.org)
 * @brief Command to manipulate wifi
 * @version 0.1
 * @date 05-10-2022
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "cmd_wifi.hpp"


static void register_lsap(void);
static void register_wificfg(void);
static void register_apinfo(void);


void prettyPrintStaConfig(espwifi::wifiStaConfig &cfg) {
    char ssid[SSID_MAX_LEN+1];
    char passwd[PASS_MAX_LEN+1];
    uint8_t bssid[6];
    char authmode_name[27];

    cfg.getSsid(ssid, SSID_MAX_LEN+1);
    cfg.getPassword(passwd, PASS_MAX_LEN+1); 
    cfg.getBssid(bssid);
    cfg.getScanThresholdAuthModeAsString(authmode_name, sizeof(authmode_name));

    std::cout   << "SSID:         " << ssid << std::endl
                << "Password:     " << passwd << std::endl 
                << "BSSID:        " << std::hex 
                                    << std::setfill('0') << std::setw(2) << static_cast<int>(bssid[0]) << ':'
                                    << std::setfill('0') << std::setw(2) << static_cast<int>(bssid[1]) << ':'
                                    << std::setfill('0') << std::setw(2) << static_cast<int>(bssid[2]) << ':'
                                    << std::setfill('0') << std::setw(2) << static_cast<int>(bssid[3]) << ':'
                                    << std::setfill('0') << std::setw(2) << static_cast<int>(bssid[4]) << ':'
                                    << std::setfill('0') << std::setw(2) << static_cast<int>(bssid[5]) 
                                    << std::endl << std::dec
                << "BSSID Set     " << (cfg.getEnforceBssid() ? "true" : "false") << std::endl
                << "Scan Method   " << (cfg.getScanMethod() == WIFI_FAST_SCAN ? "fast" : "normal") << std::endl
                << "Channel       " << static_cast<int>(cfg.getChannel()) << std::endl
                << "Listen Int.   " << cfg.getListenInterval() << std::endl
                << "Sort Method   " << (cfg.getSortMethod() == WIFI_CONNECT_AP_BY_SIGNAL ? "by signal" : "by security") << std::endl
                << "Thres. RSSI   " << static_cast<int>(cfg.getScanThresholdRSSI()) << std::endl
                << "Thres. Auth.  " << authmode_name <<  std::endl
                << "PMF Config.   " << "Capable: " << cfg.getPMFCapable() << " Required: " << cfg.getPMFRequired() << std::endl
                << "Radio Mgmt.   " << cfg.getRM() << std::endl
                << "BSS Tr. Mgmt. " << cfg.getBTM() << std::endl
                << "MBO enabled   " << cfg.getMBO() << std::endl;

}

/** register all commands in this library */
void register_wifi(void) {
    register_lsap();
    register_apinfo();
    register_wificfg();
}


/** 'lsap' command list available access points */
int lsap(int argc, char **argv) {
    ESP_LOGI(TAG_COMMAND_WIFI, "List Access Points");
    
    espwifi::wifiController Wifi;
    Wifi.scanAPs();
    espwifi::accessPointList ap_list = Wifi.getAPList();
    for (auto i : ap_list) {
        std::cout << "Next AP is: " << std::endl;
        std::cout << i << "\n";
    }

    return 0;
}

static void register_lsap(void) {
    const ch405_labs_esp_console_cmd_t cmd = {
        .command = "lsap",
        .help = "List available access points.",
        .hint = NULL,
        .func = &lsap,
        .argtable = NULL,
    };
    ESP_ERROR_CHECK( ch405_labs_esp_console_cmd_register(&cmd) );
}


/**
 * @brief Configure wifi through the serial console
 * 
 * @param argc the number of given arguments
 * @param argv the vector of the arguments (0 terminated)
 * @return 0 if successful.
 */
int wificfg(int argc, char **argv) {

    ESP_LOGI(TAG_COMMAND_WIFI, "Configure WiFi.");

    espwifi::wifiController Wifi;
    bool argerror = false;

    if(argc < 2) {
        ESP_LOGD(TAG_WIFI, "wificfg(): Insufficient arguments.");
        printf("Usage: wificmd subcommand [parameters]\n");
        printf("        --> use 'wificmd help' for further information.\n");
        return -1;
    }

    // Process subcommand
    //
    // Each subcommand can have different parameters, so we differentiate here.
    // The following subcommands are available:
    //      set         prepare a new configuration (but don't reconfigure wifi yet)
    //      update      update the wifi configuration with the set configuration 
    //                  (may disconnect from the access point)
    //      connect     connect to an access point using the current configuration
    //      apinfo      information about currently connected access point
    //      list        list saved configurations
    //      load        load previously saved configurations
    //      save        save the current configuration to permanent storage
    //      stage       stage a previously saved configuration
    if(strcasecmp(argv[1], "help") == 0) {
        // Identify parameters. Valid parameters are only other subcommands.
        if(argc == 2) {
            // General help
            // Colwidth helper:
            //      "123*56789012345*7890123456789012345678901234567890123456789012345678901234567890"
            printf( "Usage: wificmd subcommand [paramters]\n"
                    "\n"
                    "Available subcommnds:\n"
                    "   set         prepare a new configuration (but don't reconfigure wifi yet)\n"
                    "   show        show the staged configuration\n"
                    "   update      update the wifi configuration with the set configuration\n"
                    "               (may disconnect from the access point)\n"
                    "   connect     connect to an access point using the current configuration\n"
                    "   list        list saved configurations\n"
                    "   load        load a previously saved configuration\n"
                    "   save        save the current configuration to permanent storage\n"
                    "   stage       stage configuration from list of loaded configurations\n"
                    "\n"
                    );
        } else if(argc == 3) {
            // Help on a specific subcommand
            // Colwidth helper:
            //          "123*56789012345*7890123456789012345678901234567890123456789012345678901234567890"
            if(strcasecmp(argv[2], "set") == 0) {
                printf( "Usage: wificmd set {param <value>}\n"
                        "\n"
                        "Set paramters in the currently staged configuration. A staged configuration\n"
                        "does not directly affect wifi, but can be applied using other subcommands.\n"
                        "\n"
                        "Attention: A maximum of three parameter/value pairs can be given per invocation.\n"
                        "\n"
                        "Where valid parameter/value pairs are:\n"
                        "   ssid <name>      The name the access point uses as ssid. Use \"<name>\" if\n"
                        "                    the name contains spaces.\n"
                        "   pass <password>  The password to use for the access point.\n"
                        "   smethod <method> The scan method. FAST or NORMAL.\n"
                        "   bssid <bssid>    Set BSSID of the access point in the form XX:XX:XX:XX:XX:XX\n" 
                        "                    where XX is in hex. If forcebssid is set, wifi will only\n"
                        "                    connect to the access point with the set BSSID.\n"
                        "   forcebssid <0|1> If the BSSID should be enforce (1) or not (2).\n"
                        "   channel <number> Channel number of the access point.\n"
                        "   listen_iv <iv>   The listen interval.\n"
                        "   sortmtd <method> The method to sort access points for fast scanning. Can be\n"
                        "                    set by \"signal\" strength or \"security\" mode.\n"
                        "   rssithrs <num>   The threshold of the rssi signal as number.\n"
                        "   auththrs <mode>  Set the threshold for the authmode. Can be \"OPEN\", \"WEP\"\n"
                        "                    \"WPA_PSK\", \"WPA2_PSK\", \"WPA_WPA2_PSK\",\n"
                        "                    \"WPA2_ENTERPRISE\", \"WPA3_PSK\", \"WPA2_WPA3_PSK\" or \n"
                        "                    \"WAPI_PSK\".\n"
                        "   pmfcapable <0|1> If the device is protected managment frame capable (1) or\n"
                        "                    not (0).\n"
                        "   pmfreq <0|1>     If protected management frame capability is required (1) or\n"
                        "                    not (0).\n" 
                        "   rm <0|1>         Enable (1) or disable (0) radio management.\n"
                        "   btm <0|1>        Enable (1) or disable (0) BSS transistion management.\n"
                        "   mbo <0|1>        Enable (1) or disable (0) mbo.\n"
                        "   ft <0|1>         Enable (1) or disable (0) fast transition.\n"
                        "   owe <0|1>        Enable (1) or disable (0) OWE\n"
                        "   tr_dis <0|1>     Enable (1) or disable (0) transition disable\n"
                        "   sae_pwe <mode>   Set the SAE PWE mode. Can be HUNT_AND_PECK, HASH_TO_ELEMENT\n"
                        "                    or BOTH or UNSPECIFIED.\n"
                        "   fail_retry <cnt> The number <cnt> of retries to connect to the same access\n"
                        "                    point before moving to the next in the list.\n"
                        "\n"
                );
            } else if(strcasecmp(argv[2], "show") == 0) {
                // Colwidth helper:
                //      "123*56789012345*7890123456789012345678901234567890123456789012345678901234567890"
                printf( "Usage: wificmd show\n"
                        "\n"
                        "Prints the staged configuration.\n"
                        "\n"
                );
            } else if(strcasecmp(argv[2], "update") == 0) {
                // Colwidth helper:
                //      "123*56789012345*7890123456789012345678901234567890123456789012345678901234567890"
                printf( "Usage: wificmd update\n"
                        "\n"
                        "Updates the active wifi configruation with the staged configuration. This may\n"
                        "lead to a disconnect.\n"
                        "\n"
                );
            } else if(strcasecmp(argv[2], "connect") == 0) {
                // Colwidth helper:
                //      "123*56789012345*7890123456789012345678901234567890123456789012345678901234567890"
                printf( "Usage: wificmd connect\n"
                        "\n"
                        "Use the staged configuration to connect to an access point.\n"
                        "\n"
                );
            } else if(strcasecmp(argv[2], "apinfo") == 0) {
                // Colwidth helper:
                //      "123*56789012345*7890123456789012345678901234567890123456789012345678901234567890"
                printf( "Usage: wificmd apinfo\n"
                        "\n"
                        "Print information about curretly associated access point.\n"
                        "\n"
                );
            } else if(strcasecmp(argv[2], "list") == 0) {
                // Colwidth helper:
                //      "123*56789012345*7890123456789012345678901234567890123456789012345678901234567890"
                printf( "Usage: wificmd list\n"
                        "\n"
                        "List all saved configurations.\n"
                        "\n"
                );
            } else if(strcasecmp(argv[2], "load") == 0) {
                // Colwidth helper:
                //      "123*56789012345*7890123456789012345678901234567890123456789012345678901234567890"
                printf( "Usage: wifi load\n"
                        "\n"
                        "Load saved configurations from disk.\n"
                        "\n"
                );
            } else if(strcasecmp(argv[2], "save") == 0) {
                // Colwidth helper:
                //      "123*56789012345*7890123456789012345678901234567890123456789012345678901234567890"
                printf( "Usage: wificmd save <name>\n"
                        "\n"
                        "Save the staged configuration as <name>. Names can be ambigous. Every saved\n"
                        "configuration gets automatically associated a unique id as well.\n"
                        "\n"
                        "Valid value pairs are:\n"
                        "   <name>       Name of the configuration. Use \"<name>\" if name contains\n"
                        "                spaces.\n"
                        "\n"
                );
            } else if(strcasecmp(argv[2], "stage") == 0) {
                // TODO allow load by name (name <name>)
                // Colwidth helper:
                //      "123*56789012345*7890123456789012345678901234567890123456789012345678901234567890"
                printf( "Usage: wificfg stage id <id>\n"
                        "\n"
                        "Stage a saved configuration. Provide <id> to identify the configuration to laod.\n"
                        "\n"
                        "Valid key / value pairs are:\n"
                        "   id <id>     ID of the configuration to stage.\n"
                        "\n"
                );
            } else {
                printf("Invalid subcommand.\n");
                return -1;
            }

        } else {
            printf("Usage: wificmd help [subcommand]\n");
            return -1;
        }
    } else if(strcasecmp(argv[1], "lsc") == 0) {
        // Undoucmented command to load configuration, stage <id> and connect
        if(argc != 3) {
            printf("Nope, not helping you with undocumented commands.\n");
            return -1;
        }

        // Try to parse id
        size_t id;
        std::stringstream id_string(argv[2]);
        id_string >> id;
        if(!id_string) {
            printf( "ID is not a valid number (%s)\n", argv[2]);
            return -1;
        } 

        // Load configurations
        loaded_configs.clear();
        wifi_err_t err = Wifi.loadConfigs(loaded_configs);
        if(err != WIFI_OK) {
            std::cout << "Failed loading configurations (" << err << ")" << std::endl;
            return -1;
        } else {
            std::cout << "Successfully loaded configurations." << std::endl;
        }

        // Stage requested configuration
        // Check if we have some configs loaded at all
        if(loaded_configs.size() == 0) {
            printf("No config loaded.\n");
            return -1;
        }

        // Check if id is numerical
        if(id > loaded_configs.size() - 1) {
            printf("ID out of bounds.\n");
            return -1;
        }
        
        staged_config = loaded_configs.at(id);

        // Connect
        // If we are connected, disconnect
        espwifi::wifiController::state_e wifi_state = Wifi.GetState();
        if(wifi_state == espwifi::wifiController::state_e::CONNECTED ||
            wifi_state == espwifi::wifiController::state_e::CONNECTING ||
            wifi_state == espwifi::wifiController::state_e::WAITING_FOR_IP) {
            
            wifi_err_t status = Wifi.disconnect();
            if(status != ESP_OK) {
                ESP_LOGE(TAG_WIFI, "Could not disconnect from AP.");
                return -1; // TODO proper error code
            }

            // Wait at most 2.5 seconds for disconnect
            for(int i = 0; i < 5; i++) {
                wifi_state = Wifi.GetState();
                if(wifi_state != espwifi::wifiController::state_e::DISCONNECTED || 
                    wifi_state != espwifi::wifiController::state_e::READY_TO_CONNECT) {       
                    vTaskDelay(pdMS_TO_TICKS(500));
                } else {
                    break;
                }
            }

        }

        // Apply staged config to wifiController
        Wifi.setStaConfig(staged_config);

        // Connect
        wifi_err_t status = Wifi.connect();
        if(status != WIFI_OK) {
            printf("Failed to connect.\n");
        }    
    } else if(strcasecmp(argv[1], "set") == 0) {

        if(argc < 4 || argc > 8 || argc % 2 != 0) {
            printf( "Usage: wificmd set {key <value>}\n"
                    "You can specify maximum three keys per invocation.\n");
            return -1;
        }

        int arg_index = 2;
        while(arg_index < argc) {
            if(strcasecmp(argv[arg_index], "ssid") == 0) {
                arg_index++;
                staged_config.setSsid(argv[arg_index]); 
            } else if(strcasecmp(argv[arg_index], "pass") == 0) {
                arg_index++;
                staged_config.setPassword(argv[arg_index]);
            } else if(strcasecmp(argv[arg_index], "smethod") == 0) {
                arg_index++;
                if(strcasecmp(argv[arg_index], "FAST") == 0) {
                    staged_config.setScanMethod(WIFI_FAST_SCAN);
                } else if(strcasecmp(argv[arg_index], "NORMAL") == 0) {
                    staged_config.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
                } else {
                    argerror = true;
                }
            } else if(strcasecmp(argv[arg_index], "bssid") == 0) {
                arg_index++;

                // Parse argument and check validity of hex numbers
                uint8_t bssid[BSSID_LEN];
                unsigned int val;
                char separator;
                std::stringstream bssid_string(argv[arg_index]);
                for(int i = 0; i < 6; i++) {
                    bssid_string >> std::hex >> val;
                    if(!bssid_string || val > 0xFF) {
                        argerror = true;
                        break;
                    }
                    bssid[i] = val; // we know its max 8 bits
                    if(i < 5) {
                        bssid_string >> separator;
                        if(!bssid_string) {
                            argerror = true;
                            break;
                        }
                    }
                }
                if(!argerror) {
                    staged_config.setBssid(bssid);
                }
            } else if(strcasecmp(argv[arg_index], "forcebssid") == 0) {
                arg_index++;
                if(argv[arg_index][1] != 0) {
                    argerror = true;
                } else {
                    if(argv[arg_index][0] == '0') {
                        staged_config.setEnforceBssid(false);
                    } else if(argv[arg_index][0] == '1') {
                        staged_config.setEnforceBssid(true);
                    } else {
                        argerror = true;
                    }
                }
            } else if(strcasecmp(argv[arg_index], "channel") == 0) {
                arg_index++;
                unsigned int channel;
                std::stringstream channel_string(argv[arg_index]);
                channel_string >> channel;
                // TODO check if the number is actually a valid channel
                if(!channel_string || channel > 0xFF) {
                    argerror = true;
                } else {
                    staged_config.setChannel(static_cast<uint8_t>(channel));
                }
            } else if(strcasecmp(argv[arg_index], "listen_iv") == 0) {
                arg_index++;
                uint16_t interval;
                std::stringstream interval_string(argv[arg_index]);
                interval_string >> interval;
                // TODO check if the number is actually a valid listen interval
                if(!interval_string) {
                    argerror = true;
                } else {
                    staged_config.setListenInterval(interval);
                }
            } else if(strcasecmp(argv[arg_index], "sortmtd") == 0) {
                arg_index++;
                if(strcasecmp(argv[arg_index], "signal") == 0) {
                    staged_config.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);
                } else if(strcasecmp(argv[arg_index], "security") == 0) {
                    staged_config.setSortMethod(WIFI_CONNECT_AP_BY_SECURITY);
                } else {
                    argerror = true;
                }
            } else if(strcasecmp(argv[arg_index], "rssithrs") == 0) {
                arg_index++;
                unsigned int rssi_threshold;
                std::stringstream rssi_threshold_string(argv[arg_index]);
                rssi_threshold_string >> rssi_threshold;
                // TODO check if the number is actually a valid channel
                if(!rssi_threshold_string || rssi_threshold > 0xFF) {
                    argerror = true;
                } else {
                    staged_config.setScanThresholdRSSI(static_cast<uint8_t>(rssi_threshold));
                }
            } else if(strcasecmp(argv[arg_index], "auththrs") == 0) {
                arg_index++;
                if(strcasecmp(argv[arg_index], "open") == 0) {
                    staged_config.setScanThresholdAuthMode(WIFI_AUTH_OPEN);
                } else if(strcasecmp(argv[arg_index], "wep") == 0) {
                    staged_config.setScanThresholdAuthMode(WIFI_AUTH_WEP);
                } else if(strcasecmp(argv[arg_index], "wpa_psk") == 0) {
                    staged_config.setScanThresholdAuthMode(WIFI_AUTH_WPA_PSK);
                } else if(strcasecmp(argv[arg_index], "wpa2_psk") == 0) {
                    staged_config.setScanThresholdAuthMode(WIFI_AUTH_WPA2_PSK);
                } else if(strcasecmp(argv[arg_index], "wpa_wpa2_psk") == 0) {
                    staged_config.setScanThresholdAuthMode(WIFI_AUTH_WPA_WPA2_PSK);
                } else if(strcasecmp(argv[arg_index], "wpa2_enterprise") == 0) {
                    staged_config.setScanThresholdAuthMode(WIFI_AUTH_WPA2_ENTERPRISE);
                } else if(strcasecmp(argv[arg_index], "wpa3_psk") == 0) {
                    staged_config.setScanThresholdAuthMode(WIFI_AUTH_WPA3_PSK);
                } else if(strcasecmp(argv[arg_index], "wpa2_wpa3_psk") == 0) {
                    staged_config.setScanThresholdAuthMode(WIFI_AUTH_WPA2_WPA3_PSK);
                } else if(strcasecmp(argv[arg_index], "wapi_psk") == 0) {
                    staged_config.setScanThresholdAuthMode(WIFI_AUTH_WAPI_PSK);
                } else {
                    argerror = true;
                }
            } else if(strcasecmp(argv[arg_index], "pmfcapable") == 0) {
                arg_index++;
                if(argv[arg_index][1] != 0) {
                    argerror = true;
                } else {
                    if(argv[arg_index][0] == '0') {
                        staged_config.setPMFCapable(false);
                    } else if(argv[arg_index][0] == '1') {
                        staged_config.setPMFCapable(true);
                    } else {
                        argerror = true;
                    }
                }
            } else if(strcasecmp(argv[arg_index], "pmfreq") == 0) {
                arg_index++;
                if(argv[arg_index][1] != 0) {
                    argerror = true;
                } else {
                    if(argv[arg_index][0] == '0') {
                        staged_config.setPMFRequired(false);
                    } else if(argv[arg_index][0] == '1') {
                        staged_config.setPMFRequired(true);
                    } else {
                        argerror = true;
                    }
                }
            } else if(strcasecmp(argv[arg_index], "rm") == 0) {
                arg_index++;
                if(argv[arg_index][1] != 0) {
                    argerror = true;
                } else {
                    if(argv[arg_index][0] == '0') {
                        staged_config.setRM(false);
                    } else if(argv[arg_index][0] == '1') {
                        staged_config.setRM(true);
                    } else {
                        argerror = true;
                    }
                }
            } else if(strcasecmp(argv[arg_index], "btm") == 0) {
                arg_index++;
                if(argv[arg_index][1] != 0) {
                    argerror = true;
                } else {
                    if(argv[arg_index][0] == '0') {
                        staged_config.setBTM(false);
                    } else if(argv[arg_index][0] == '1') {
                        staged_config.setBTM(true);
                    } else {
                        argerror = true;
                    }
                }
            } else if(strcasecmp(argv[arg_index], "mbo") == 0) {
                arg_index++;
                if(argv[arg_index][1] != 0) {
                    argerror = true;
                } else {
                    if(argv[arg_index][0] == '0') {
                        staged_config.setMBO(false);
                    } else if(argv[arg_index][0] == '1') {
                        staged_config.setMBO(true);
                    } else {
                        argerror = true;
                    }
                }
            } else if(strcasecmp(argv[arg_index], "ft") == 0) {
                arg_index++;
                if(argv[arg_index][1] != 0) {
                    argerror = true;
                } else {
                    if(argv[arg_index][0] == '0') {
                        staged_config.setFTEnabled(false);
                    } else if(argv[arg_index][0] == '1') {
                        staged_config.setFTEnabled(true);
                    } else {
                        argerror = true;
                    }
                }
            } else if(strcasecmp(argv[arg_index], "owe") == 0) {
                arg_index++;
                if(argv[arg_index][1] != 0) {
                    argerror = true;
                } else {
                    if(argv[arg_index][0] == '0') {
                        staged_config.setOWEEnabled(false);
                    } else if(argv[arg_index][0] == '1') {
                        staged_config.setOWEEnabled(true);
                    } else {
                        argerror = true;
                    }
                }
            } else if(strcasecmp(argv[arg_index], "tr_dis") == 0) {
                arg_index++;
                if(argv[arg_index][1] != 0) {
                    argerror = true;
                } else {
                    if(argv[arg_index][0] == '0') {
                        staged_config.setTransitionDisable(false);
                    } else if(argv[arg_index][0] == '1') {
                        staged_config.setTransitionDisable(true);
                    } else {
                        argerror = true;
                    }
                }
            } else if(strcasecmp(argv[arg_index], "sae_pwe") == 0) {
                arg_index++;
                if(strcasecmp(argv[arg_index], "unspecified") == 0) {
                    staged_config.setSaePweH2e(WPA3_SAE_PWE_UNSPECIFIED);
                } else if(strcasecmp(argv[arg_index], "hunt_and_peck") == 0) {
                    staged_config.setSaePweH2e(WPA3_SAE_PWE_HUNT_AND_PECK);
                } else if(strcasecmp(argv[arg_index], "hash_to_element") == 0) {
                    staged_config.setSaePweH2e(WPA3_SAE_PWE_HASH_TO_ELEMENT);
                } else if(strcasecmp(argv[arg_index], "both") == 0) {
                    staged_config.setSaePweH2e(WPA3_SAE_PWE_BOTH);
                } else {
                    argerror = true;
                }
            } else if(strcasecmp(argv[arg_index], "fail_retry") == 0) {
                arg_index++;
                unsigned int fail_retry;
                std::stringstream fail_retry_string(argv[arg_index]);
                fail_retry_string >> fail_retry;
                if(!fail_retry_string || fail_retry > 0xFF) {
                    argerror = true;
                } else {
                    staged_config.setFailureRetryCount(static_cast<uint8_t>(fail_retry));
                }
            } else {
                argerror = true;
            }

            if(argerror) {
                printf("%s: %s\n", WIFICMD_INVALID_ARG_STRING, argv[arg_index]);
                return -1;
            }

            arg_index++;
        }
    } else if(strcasecmp(argv[1], "show") == 0) {
        prettyPrintStaConfig(staged_config);
    } else if(strcasecmp(argv[1], "update") == 0) {
        //TODO test
        ESP_LOGV(TAG_WIFI, "Updating wifi config with staged_config...");
        Wifi.setStaConfig(staged_config);
    } else if(strcasecmp(argv[1], "connect") == 0) {
        if(argc > 2) {
            argerror = true;
        } else {
            // If we are connected, disconnect
            espwifi::wifiController::state_e wifi_state = Wifi.GetState();
            if(wifi_state == espwifi::wifiController::state_e::CONNECTED ||
                wifi_state == espwifi::wifiController::state_e::CONNECTING ||
                wifi_state == espwifi::wifiController::state_e::WAITING_FOR_IP) {
                
                wifi_err_t status = Wifi.disconnect();
                if(status != ESP_OK) {
                    ESP_LOGE(TAG_WIFI, "Could not disconnect from AP.");
                    return -1; // TODO proper error code
                }

                // Wait at most 2.5 seconds for disconnect
                for(int i = 0; i < 5; i++) {
                    wifi_state = Wifi.GetState();
                    if(wifi_state != espwifi::wifiController::state_e::DISCONNECTED || 
                        wifi_state != espwifi::wifiController::state_e::READY_TO_CONNECT) {       
                        vTaskDelay(pdMS_TO_TICKS(500));
                    } else {
                        break;
                    }
                }

            }

            // Apply staged config to wifiController
            Wifi.setStaConfig(staged_config);

            // Connect
            wifi_err_t status = Wifi.connect();
            if(status != WIFI_OK) {
                printf("Failed to connect.\n");
            }
        }
    } else if(strcasecmp(argv[1], "list") == 0) {
        bool verbose = false;
        if(argc == 3) {
            if(strcasecmp(argv[2], "-v") == 0 || strcasecmp(argv[2], "--verbose") == 0) {
                verbose = true;
            } else {
                printf("%s: %s\n", WIFICMD_INVALID_ARG_STRING, argv[2]);
                return -1;
            }
        } else if(argc != 2) {
            printf("%s\n", WIFICMD_INVALID_ARG_STRING);
            return -1;
        }
        // List all the available configurations
        wifi_err_t err = WIFI_OK;
        std::vector<espwifi::wifiStaConfig>::iterator iter;
        char ssid[SSID_MAX_LEN];
        for(iter = loaded_configs.begin(); iter < loaded_configs.end(); iter++) {
            err = iter->getSsid(ssid, SSID_MAX_LEN);
            if(err != WIFI_OK) {
                ESP_LOGE(TAG_WIFI, "Failed to read ssid of id %d", (iter - loaded_configs.begin()));
                return -1;
            }
            std::cout << std::setw(2) << (iter - loaded_configs.begin()) << ": " << ssid << std::endl; 
            if(verbose) {
                prettyPrintStaConfig(*iter);
                std::cout << std::endl;
            }
        }
    } else if(strcasecmp(argv[1], "load") == 0) {
        loaded_configs.clear();
        wifi_err_t err = Wifi.loadConfigs(loaded_configs);
        if(err != WIFI_OK) {
            std::cout << "Failed loading configurations (" << err << ")" << std::endl;
            return -1;
        } else {
            std::cout << "Successfully loaded configurations." << std::endl;
        }
    } else if(strcasecmp(argv[1], "save") == 0) {
        wifi_err_t err = Wifi.saveConfig(staged_config);
        if(err == WIFI_OK) {
            std::cout << "Successfully saved configuration." << std::endl;
        } else {
            std::cout << "Failed to save configruation (" << err << ")" << std::endl;
            return -1;
        }
    } else if(strcasecmp(argv[1], "stage") == 0) {
        // Check number of arguments
        if(argc != 3) {
            printf( "Invalid arguments.\n"
                    "\n"
                    "Usage: wificfg stage <id>\n"
            );
            return -1;
        }

        // Check if we have some configs loaded at all
        if(loaded_configs.size() == 0) {
            printf("No config loaded.\n");
            return -1;
        }

        // Check if id is numerical
        size_t id;
        std::stringstream id_string(argv[2]);
        id_string >> id;
        if(!id_string) {
            printf( "ID is not a valid number (%s)\n", argv[2]);
            return -1;
        } else {
            if(id > loaded_configs.size() - 1) {
                printf("ID out of bounds.\n");
                return -1;
            }
        }
        staged_config = loaded_configs.at(id);
    } else {
        // Invalid subcommand
        argerror = true;
    }

    if(argerror) {
        printf( "Usage: wificmd subcommand [parameters]\n"
                "       --> use 'wificfg help' for further information.\n"
                "\n");
        return -1;
    }

    // All good and done
    return 0;
}

/**
 * @brief Register the 'wificfg' command to the esp_console
 */
static void register_wificfg(void) {
    const ch405_labs_esp_console_cmd_t cmd = {
        .command = "wificfg",
        .help = "Configure wifi",
        .hint = NULL,
        .func = &wificfg,
        .argtable = NULL,
    };
    ESP_ERROR_CHECK( ch405_labs_esp_console_cmd_register(&cmd) );
}

/**
 * @brief show iformation about the connected access point in the console
 * 
 * @param argc the number of given arguments
 * @param argv the vector with the arguments
 * @return 
 */
int apinfo(int argc, char **argv) {
    espwifi::wifiController Wifi;
    espwifi::accessPoint ap;
    wifi_err_t status = Wifi.staGetAPInfo(ap);
    if(status == WIFI_OK) {
        std::cout << ap << std::endl;
    } else if(status == WIFI_NOT_CONNECTED) {
        std::cout << "Error: WiFi not connected to any accesspoint." << std::endl;
    } else {
        std::cout << "Error: WiFi not initialised properly." << std::endl;
    }

    return 0;
}

/**
 * @brief Register apinfo command to esp_console
 */
static void register_apinfo(void) {
    const ch405_labs_esp_console_cmd_t cmd = {
        .command = "apinfo",
        .help = "Show information about connected access point.",
        .hint = NULL,
        .func = &apinfo,
        .argtable = NULL,
    };
    ESP_ERROR_CHECK( ch405_labs_esp_console_cmd_register(&cmd) );
}