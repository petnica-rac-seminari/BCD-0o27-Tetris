#pragma once
#include <gfx.hpp>

//extern const uint8_t mouse_8x8_mask[];
//extern const uint16_t mouse_8x8_data[];

//#ifdef MOUSE_8x8_IMPLEMENTATION

const uint8_t mouse_8x8_mask[] = {
	0x00, 0x0C, 0x1E, 0x7F, 0xDE, 0x0C, 0x00, 0x00
};

const uint16_t mouse_8x8_data[] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  // Line 1
    0x0000, 0x0000, 0x0000, 0x0000, 0xAA5A, 0xAA5A, 0x0000, 0x0000,  // Line 2
    0x0000, 0x0000, 0x0000, 0xAA5A, 0x4D6B, 0xDF64, 0xAA5A, 0x0000,  // Line 3
    0x0000, 0xAA5A, 0xAA5A, 0x4D6B, 0x4D6B, 0xF083, 0xF083, 0xAA5A,  // Line 4
    0xAA5A, 0xAA5A, 0x0000, 0xAA5A, 0x4D6B, 0xDF64, 0xAA5A, 0x0000,  // Line 5
    0x0000, 0x0000, 0x0000, 0x0000, 0xAA5A, 0xAA5A, 0x0000, 0x0000,  // Line 6
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  // Line 7
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,  // Line 8
};
//#endif //MOUSE_8x8_IMPLEMENTATION