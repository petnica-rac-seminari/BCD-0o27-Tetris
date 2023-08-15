#pragma once
#include <gfx.hpp>

//extern const uint8_t snake_head_16x16_mask[];
//extern const uint8_t snake_head_16x16_data[];
//extern const uint8_t *snake_body_16x16_mask; 
//extern const uint8_t snake_body_16x16_data[];

//#ifdef SNAKE_16x16_IMPLEMENTATION

const uint8_t snake_head_16x16_mask[] = {
	0x03, 0xC0, // Line 1
    0x03, 0xC0, // Line 2
    0x0F, 0xF0, // Line 3
    0x0F, 0xF0, // Line 4
    0x3F, 0xFC, // Line 5
    0x3F, 0xFC, // Line 6
    0xFF, 0xFF, // Line 7
    0xFF, 0xFF, // Line 8
    0xFF, 0xFF, // Line 9
    0xFF, 0xFF, // Line 10
    0x3F, 0xFC, // Line 11
    0x3F, 0xFC, // Line 12
    0x0F, 0xF0, // Line 13
    0x0F, 0xF0, // Line 14
    0x03, 0xC0, // Line 15
    0x03, 0xC0, // Line 16
};

const uint8_t snake_head_16x16_data[] {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // Line 1
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // Line 2
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x34, 0xAD, 0x34, 0xAD, 0x34, 0xAD, 0x34, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // Line 3
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x34, 0xAD, 0x34, 0xAD, 0x34, 0xAD, 0x34, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // Line 4   
    0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x34, 0xAD, 0x34, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x01, 0x3E, 0x01, 0x3E, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF,   // Line 5
    0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x34, 0xAD, 0x34, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x01, 0x3E, 0x01, 0x3E, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF,   // Line 6
    0x4B, 0x45, 0x4B, 0x45, 0x34, 0xAD, 0x34, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x6D, 0xE6, 0x6D, 0xE6, 0x34, 0xAD, 0x34, 0xAD, 0x4B, 0x45, 0x4B, 0x45,   // Line 7
    0x4B, 0x45, 0x4B, 0x45, 0x34, 0xAD, 0x34, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x6D, 0xE6, 0x6D, 0xE6, 0x34, 0xAD, 0x34, 0xAD, 0x4B, 0x45, 0x4B, 0x45,   // Line 8
    0x4B, 0x45, 0x4B, 0x45, 0x34, 0xAD, 0x34, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x6D, 0xE6, 0x6D, 0xE6, 0x34, 0xAD, 0x34, 0xAD, 0x4B, 0x45, 0x4B, 0x45,   // Line 9
    0x4B, 0x45, 0x4B, 0x45, 0x34, 0xAD, 0x34, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x6D, 0xE6, 0x6D, 0xE6, 0x34, 0xAD, 0x34, 0xAD, 0x4B, 0x45, 0x4B, 0x45,   // Line 10 
    0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x34, 0xAD, 0x34, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x01, 0x3E, 0x01, 0x3E, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF,   // Line 11
    0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x34, 0xAD, 0x34, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x01, 0x3E, 0x01, 0x3E, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF,   // Line 12
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x34, 0xAD, 0x34, 0xAD, 0x34, 0xAD, 0x34, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // Line 13
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x34, 0xAD, 0x34, 0xAD, 0x34, 0xAD, 0x34, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // Line 14   
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // Line 15
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // Line 16
};

//const uint8_t *snake_body_16x16_mask = snake_head_16x16_mask;

const uint8_t snake_body_16x16_data[] {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // Line 1
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // Line 2
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x34, 0xAD, 0x34, 0xAD, 0x34, 0xAD, 0x34, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // Line 3
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x34, 0xAD, 0x34, 0xAD, 0x34, 0xAD, 0x34, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // Line 4   
    0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x34, 0xAD, 0x34, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x34, 0xAD, 0x34, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF,   // Line 5
    0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x34, 0xAD, 0x34, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x34, 0xAD, 0x34, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF,   // Line 6
    0x4B, 0x45, 0x4B, 0x45, 0x34, 0xAD, 0x34, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x6D, 0xE6, 0x6D, 0xE6, 0x34, 0xAD, 0x34, 0xAD, 0x4B, 0x45, 0x4B, 0x45,   // Line 7
    0x4B, 0x45, 0x4B, 0x45, 0x34, 0xAD, 0x34, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x6D, 0xE6, 0x6D, 0xE6, 0x34, 0xAD, 0x34, 0xAD, 0x4B, 0x45, 0x4B, 0x45,   // Line 8
    0x4B, 0x45, 0x4B, 0x45, 0x34, 0xAD, 0x34, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x6D, 0xE6, 0x6D, 0xE6, 0x34, 0xAD, 0x34, 0xAD, 0x4B, 0x45, 0x4B, 0x45,   // Line 9
    0x4B, 0x45, 0x4B, 0x45, 0x34, 0xAD, 0x34, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x6D, 0xE6, 0x6D, 0xE6, 0x34, 0xAD, 0x34, 0xAD, 0x4B, 0x45, 0x4B, 0x45,   // Line 10 
    0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x34, 0xAD, 0x34, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x34, 0xAD, 0x34, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF,   // Line 11
    0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x34, 0xAD, 0x34, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x34, 0xAD, 0x34, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF,   // Line 12
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x34, 0xAD, 0x34, 0xAD, 0x34, 0xAD, 0x34, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // Line 13
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x34, 0xAD, 0x34, 0xAD, 0x34, 0xAD, 0x34, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // Line 14   
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // Line 15
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // Line 16
};

//#endif //SNAKE_HEAD_32x32_IMPLEMENTATION