#include "led_patterns.h"

void bootupPattern(hUpdateLED update_led, void *arg) {
    ledStates new_state;

    rgbLEDPixel red = {
        .red = 0x01,
        .green = 0x00,
        .blue = 0x00
    };

    rgbLEDPixel green = {
        .red = 0x00,
        .green = 0x01,
        .blue = 0x00
    };

    rgbLEDPixel blue = {
        .red = 0x00,
        .green = 0x00,
        .blue = 0x01
    };

    for(u_int32_t i = 0; i < 100; i+=1) {
        int j = i % 18;

        // Adjust intensity of leds.
        red.red = i/8;
        green.green = i/8;
        blue.blue = i/8;

        switch(j) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
                new_state.led[0] = red;
                new_state.led[1] = green;
                new_state.led[2] = blue;
                new_state.led[3] = red;
                new_state.led[4] = green;
                new_state.led[5] = blue;
                break;
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
                new_state.led[0] = blue;         
                new_state.led[1] = red;
                new_state.led[2] = green;
                new_state.led[3] = blue;
                new_state.led[4] = red;
                new_state.led[5] = green;
                break;
            case 12:
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
                new_state.led[0] = green;     
                new_state.led[1] = blue;
                new_state.led[2] = red;
                new_state.led[3] = green;
                new_state.led[4] = blue;
                new_state.led[5] = red;
                break;

        }

        update_led(new_state);
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void testPattern(hUpdateLED update_led, void *arg) {
    ledStates new_state;

    rgbLEDPixel red = {
        .red = 0x01,
        .green = 0x00,
        .blue = 0x00
    };

    rgbLEDPixel green = {
        .red = 0x00,
        .green = 0x01,
        .blue = 0x00
    };

    rgbLEDPixel blue = {
        .red = 0x00,
        .green = 0x00,
        .blue = 0x01
    };

    for(u_int32_t i = 0; i < 255; i++) {
        int j = i % 18;

        // Adjust intensity of leds.
        red.red = i/8;
        green.green = i/8;
        blue.blue = i/8;

        switch(j) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
                new_state.led[0] = green;     // 0xFF0000;
                new_state.led[1] = red;
                new_state.led[2] = blue;
                new_state.led[3] = green;
                new_state.led[4] = red;
                new_state.led[5] = blue;
                break;
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
                new_state.led[0] = blue;          // 0xFF0000;
                new_state.led[1] = green;
                new_state.led[2] = red;
                new_state.led[3] = blue;
                new_state.led[4] = green;
                new_state.led[5] = red;
                break;
            case 12:
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
                new_state.led[0] = red;     // 0xFF0000;
                new_state.led[1] = blue;
                new_state.led[2] = green;
                new_state.led[3] = red;
                new_state.led[4] = blue;
                new_state.led[5] = green;
                break;

        }

        update_led(new_state);
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}