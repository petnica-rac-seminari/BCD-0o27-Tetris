#pragma once
#include <gfx.hpp>

const gfx::size16 startBackground_size = { 32, 32 };

const uint8_t startBackground_mask[] {
	0x00, 0x0F, 0xF0, 0x00, 
    0x00, 0x0F, 0xF0, 0x00,
    0x00, 0x0F, 0xF0, 0x00,
    0x00, 0x0F, 0xF0, 0x00,

    0x00, 0xFF, 0xFF, 0x00,
    0x00, 0xFF, 0xFF, 0x00,
    0x00, 0xFF, 0xFF, 0x00,
    0x00, 0xFF, 0xFF, 0x00,

    0x0F, 0xFF, 0xFF, 0xF0,
    0x0F, 0xFF, 0xFF, 0xF0,
    0x0F, 0xFF, 0xFF, 0xF0,
    0x0F, 0xFF, 0xFF, 0xF0,

    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,

    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,

    0x0F, 0xFF, 0xFF, 0xF0,
    0x0F, 0xFF, 0xFF, 0xF0,
    0x0F, 0xFF, 0xFF, 0xF0,
    0x0F, 0xFF, 0xFF, 0xF0,

    0x00, 0xFF, 0xFF, 0x00,
    0x00, 0xFF, 0xFF, 0x00,
    0x00, 0xFF, 0xFF, 0x00,
    0x00, 0xFF, 0xFF, 0x00,

    0x00, 0x0F, 0xF0, 0x00, 
    0x00, 0x0F, 0xF0, 0x00,
    0x00, 0x0F, 0xF0, 0x00,
    0x00, 0x0F, 0xF0, 0x00
};

const uint8_t startBackground_data[] {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45,   // 0x0010 (16) pixels
    0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // 0x0020 (32) pixels
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45,   // 0x0030 (48) pixels
    0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // 0x0040 (64) pixels
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45,   // 0x0050 (80) pixels
    0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // 0x0060 (96) pixels
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45,   // 0x0070 (112) pixels
    0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // 0x0080 (128) pixels
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD,   // 0x0090 (144) pixels
    0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // 0x00A0 (160) pixels
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD,   // 0x00B0 (176) pixels
    0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // 0x00C0 (192) pixels
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD,   // 0x00D0 (208) pixels
    0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // 0x00E0 (224) pixels
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD,   // 0x00F0 (240) pixels
    0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // 0x0100 (256) pixels
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6,   // 0x0110 (272) pixels
    0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x09, 0x3E, 0x09, 0x3E, 0x09, 0x3E, 0x09, 0x3E, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // 0x0120 (288) pixels
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6,   // 0x0130 (304) pixels
    0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x09, 0x3E, 0x09, 0x3E, 0x09, 0x3E, 0x09, 0x3E, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // 0x0140 (320) pixels
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6,   // 0x0150 (336) pixels
    0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x09, 0x3E, 0x09, 0x3E, 0x09, 0x3E, 0x09, 0x3E, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // 0x0160 (352) pixels
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6,   // 0x0170 (368) pixels
    0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x09, 0x3E, 0x09, 0x3E, 0x09, 0x3E, 0x09, 0x3E, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // 0x0180 (384) pixels
    0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A,   // 0x0190 (400) pixels
    0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45,   // 0x01A0 (416) pixels
    0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A,   // 0x01B0 (432) pixels
    0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45,   // 0x01C0 (448) pixels
    0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A,   // 0x01D0 (464) pixels
    0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45,   // 0x01E0 (480) pixels
    0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A,   // 0x01F0 (496) pixels
    0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45,   // 0x0200 (512) pixels
    0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A,   // 0x0210 (528) pixels
    0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45,   // 0x0220 (544) pixels
    0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A,   // 0x0230 (560) pixels
    0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45,   // 0x0240 (576) pixels
    0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A,   // 0x0250 (592) pixels
    0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45,   // 0x0260 (608) pixels
    0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A,   // 0x0270 (624) pixels
    0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x9F, 0x2A, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45,   // 0x0280 (640) pixels
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6,   // 0x0290 (656) pixels
    0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x09, 0x3E, 0x09, 0x3E, 0x09, 0x3E, 0x09, 0x3E, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // 0x02A0 (672) pixels
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6,   // 0x02B0 (688) pixels
    0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x09, 0x3E, 0x09, 0x3E, 0x09, 0x3E, 0x09, 0x3E, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // 0x02C0 (704) pixels
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6,   // 0x02D0 (720) pixels
    0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x09, 0x3E, 0x09, 0x3E, 0x09, 0x3E, 0x09, 0x3E, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // 0x02E0 (736) pixels
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6,   // 0x02F0 (752) pixels
    0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x6D, 0xE6, 0x09, 0x3E, 0x09, 0x3E, 0x09, 0x3E, 0x09, 0x3E, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // 0x0300 (768) pixels
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD,   // 0x0310 (784) pixels
    0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // 0x0320 (800) pixels
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD,   // 0x0330 (816) pixels
    0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // 0x0340 (832) pixels
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD,   // 0x0350 (848) pixels
    0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // 0x0360 (864) pixels
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD,   // 0x0370 (880) pixels
    0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x3C, 0xAD, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // 0x0380 (896) pixels
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45,   // 0x0390 (912) pixels
    0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // 0x03A0 (928) pixels
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45,   // 0x03B0 (944) pixels
    0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // 0x03C0 (960) pixels
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45,   // 0x03D0 (976) pixels
    0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // 0x03E0 (992) pixels
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45,   // 0x03F0 (1008) pixels
    0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0x4B, 0x45, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   // 0x0400 (1024) pixels
};