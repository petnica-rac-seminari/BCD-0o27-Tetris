#include "../include/bcd_system.hpp"

bcd_system bcd_sys;

bool bcd_system::displaySupport() {
    return capabilities.display;
}

void bcd_system::setDisplaySupport(bool s) {
    capabilities.display = s;
}

lcd_type &bcd_system::getDisplay() {
    return lcd;
}

espidf::spi_master &bcd_system::getSpiHost() {
    return spi_host;
}

bool bcd_system::ledSupport() {
    return capabilities.led;
}

void bcd_system::setLedSupport(bool s) {
    capabilities.led = s;
}

ledDriver &bcd_system::getLedDriver() {
    return led;
}

bool bcd_system::spiffsSupport() {
    return capabilities.fs_spiffs;
}

void bcd_system::setSpiffsSupport(bool s) {
    capabilities.fs_spiffs = s;
}

bool bcd_system::controllerSupport() {
    return capabilities.controller;
}

void bcd_system::setControllerSupport(bool s) {
    capabilities.controller = s;
}

controllerDriver &bcd_system::getControllerDriver() {
    return controller;
}

bool bcd_system::consoleSupport() {
    return capabilities.console;
}

void bcd_system::setConsoleSupport(bool s) {
    capabilities.console = s;
}

espconsole::consoleController &bcd_system::getConsoleController() {
    return console;
}

bool bcd_system::wifiSupport() {
    return capabilities.wlan;
}

void bcd_system::setWifiSupport(bool s) {
    capabilities.wlan = s;
}

espwifi::wifiController &bcd_system::getWifiController() {
    return wifi;
}
