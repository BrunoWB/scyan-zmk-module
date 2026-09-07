/* Auto-generated 2-Atlas spritesheet architecture for Corne vertical OLED display */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define DISPLAY_VIRTUAL_WIDTH  32
#define DISPLAY_VIRTUAL_HEIGHT 128
#define DISPLAY_HW_WIDTH       128
#define DISPLAY_HW_HEIGHT      32

/* Sprite slice descriptor */
struct sprite_slice {
    uint8_t x;
    uint8_t y;
    uint8_t width;
    uint8_t height;
};

/* Symbol identifiers */
enum symbol_id {
    SYMBOL_USB,
    SYMBOL_BLUETOOTH,
    SYMBOL_BATTERY_FRAME,
    SYMBOL_SPLIT_CONNECTED,
    SYMBOL_SPLIT_DISCONNECTED,
    SYMBOL_ARROW_HEAD,
    SYMBOL_ARROW_DOT,
    SYMBOL_BRACKET_LAYER_0,
    SYMBOL_BRACKET_LAYER_1,
    SYMBOL_BRACKET_LAYER_2,
    SYMBOL_BRACKET_LAYER_3,
    SYMBOL_SKULL_LAYER_0,
    SYMBOL_SKULL_LAYER_1,
    SYMBOL_SKULL_LAYER_2,
    SYMBOL_SKULL_LAYER_3,
    SYMBOL_COUNT,
};

#define SYMBOL_BRACKET_LAYER(idx) ((enum symbol_id)(SYMBOL_BRACKET_LAYER_0 + ((idx) & 3)))
#define SYMBOL_SKULL_LAYER(idx)   ((enum symbol_id)(SYMBOL_SKULL_LAYER_0 + ((idx) & 3)))

/* Spritesheet 1: Symbols & Icons Atlas (128x34, 1bpp, 16 bytes stride) */
#define SYMBOLS_ATLAS_WIDTH  128
#define SYMBOLS_ATLAS_HEIGHT 34
#define SYMBOLS_ATLAS_STRIDE 16

static const uint8_t SYMBOLS_ATLAS[34 * 16] = {
    0x18, 0x00, 0x06, 0x06, 0x00, 0x01, 0x81, 0x80, 0x00, 0x60, 0x60, 0x00, 0x18, 0x08, 0x00, 0x3C, // Row 0
    0x3C, 0x00, 0x0F, 0x0F, 0x00, 0x03, 0xC3, 0xC0, 0x00, 0xF0, 0xF0, 0x00, 0x3C, 0x08, 0x01, 0x28, // Row 1
    0x6E, 0x44, 0x1D, 0x9B, 0x91, 0x07, 0x66, 0xE4, 0x41, 0xD9, 0xB9, 0x10, 0x76, 0x10, 0x00, 0xB0, // Row 2
    0x67, 0x6F, 0xB9, 0x99, 0xDB, 0xEE, 0x66, 0x76, 0xFB, 0x99, 0x9D, 0xBE, 0xE6, 0x20, 0x00, 0x40, // Row 3
    0x63, 0xBE, 0xF1, 0x98, 0xEF, 0xBC, 0x66, 0x3B, 0xEF, 0x19, 0x8E, 0xFB, 0xC6, 0x40, 0x03, 0xBC, // Row 4
    0x61, 0x7F, 0x61, 0x98, 0x5F, 0xD8, 0x66, 0x17, 0xF6, 0x19, 0x85, 0xFD, 0x86, 0x40, 0x02, 0xA0, // Row 5
    0x37, 0xBB, 0x7B, 0x0D, 0xEE, 0xDE, 0xC3, 0x7B, 0xB7, 0xB0, 0xDE, 0xED, 0xEC, 0x47, 0xE3, 0x20, // Row 6
    0x1F, 0xFC, 0xFE, 0x07, 0xFF, 0x3F, 0x81, 0xFF, 0xCF, 0xE0, 0x7F, 0xF3, 0xF8, 0x2D, 0x72, 0x20, // Row 7
    0x7F, 0xFF, 0xFF, 0x9F, 0xFF, 0xFF, 0xE7, 0xFF, 0xFF, 0xF9, 0xFF, 0xFF, 0xFE, 0x1E, 0xB0, 0x00, // Row 8
    0x3C, 0xDE, 0xCF, 0x0F, 0x37, 0xB3, 0xC3, 0xCD, 0xEC, 0xF0, 0xF3, 0x7B, 0x3C, 0x07, 0xE0, 0x00, // Row 9
    0x14, 0x2D, 0x0A, 0x05, 0x0B, 0x42, 0x81, 0x42, 0xD0, 0xA0, 0x50, 0xB4, 0x28, 0x00, 0x00, 0x00, // Row 10
    0x7E, 0xDE, 0xDF, 0x9F, 0x97, 0x97, 0xE7, 0xE9, 0xE9, 0xF9, 0xF9, 0x7A, 0x7E, 0x08, 0x82, 0x00, // Row 11
    0xFB, 0x1E, 0x37, 0xFE, 0xC7, 0x8D, 0xFF, 0xB1, 0xE3, 0x7F, 0xEC, 0x78, 0xDF, 0x14, 0x43, 0x00, // Row 12
    0x29, 0xDE, 0xE5, 0x0A, 0x77, 0xB9, 0x42, 0x9D, 0xEE, 0x50, 0xA7, 0x7B, 0x94, 0x22, 0x23, 0x88, // Row 13
    0x6C, 0x7F, 0x8D, 0x9B, 0x1F, 0xE3, 0x66, 0xC7, 0xF8, 0xD9, 0xB1, 0xFE, 0x36, 0x41, 0x13, 0x00, // Row 14
    0xF7, 0xFF, 0xFB, 0xFD, 0xFF, 0xFE, 0xFF, 0x7F, 0xFF, 0xBF, 0xDF, 0xFF, 0xEF, 0x88, 0x8A, 0x00, // Row 15
    0x7F, 0xFF, 0xFB, 0x1F, 0xFF, 0xFE, 0xC7, 0xFF, 0xFF, 0xB1, 0xFF, 0xFF, 0xEC, 0x44, 0x10, 0x00, // Row 16
    0x1B, 0xE1, 0xF7, 0x06, 0xF8, 0x7D, 0xC1, 0xBE, 0x1F, 0x70, 0x6F, 0x87, 0xDC, 0x22, 0x20, 0x00, // Row 17
    0x1F, 0x61, 0xBE, 0x07, 0xD8, 0x6F, 0x81, 0xF6, 0x1B, 0xE0, 0x7D, 0x86, 0xF8, 0x11, 0x40, 0x00, // Row 18
    0x17, 0x73, 0xBA, 0x05, 0xDC, 0xEE, 0x81, 0x77, 0x3B, 0xA0, 0x5D, 0xCE, 0xE8, 0x08, 0x80, 0x00, // Row 19
    0x01, 0xBB, 0x60, 0x00, 0x6E, 0xD8, 0x00, 0x1B, 0xB6, 0x00, 0x06, 0xED, 0x80, 0x00, 0x00, 0x00, // Row 20
    0x00, 0x40, 0x80, 0x00, 0x10, 0x20, 0x00, 0x04, 0x08, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, // Row 21
    0x00, 0x3F, 0x00, 0x00, 0x0F, 0xC0, 0x00, 0x03, 0xF0, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, // Row 22
    0x06, 0x85, 0x80, 0x1A, 0x1E, 0x00, 0x78, 0x58, 0x01, 0xE1, 0xE0, 0xFF, 0xFF, 0x00, 0x80, 0x00, // Row 23
    0x09, 0x02, 0x40, 0x24, 0x0F, 0x00, 0xF0, 0x24, 0x03, 0xC0, 0xF0, 0x80, 0x01, 0x01, 0x40, 0x00, // Row 24
    0x12, 0x01, 0x20, 0x48, 0x07, 0x81, 0xE0, 0x12, 0x07, 0x80, 0x78, 0x80, 0x00, 0x82, 0x20, 0x00, // Row 25
    0x24, 0x00, 0x90, 0x90, 0x03, 0xC3, 0xC0, 0x09, 0x0F, 0x00, 0x3C, 0x80, 0x00, 0x84, 0x10, 0x00, // Row 26
    0x48, 0x00, 0x49, 0x20, 0x01, 0xE7, 0x80, 0x04, 0x9E, 0x00, 0x1E, 0x80, 0x00, 0x88, 0x08, 0x00, // Row 27
    0x90, 0x00, 0x26, 0x40, 0x00, 0xFF, 0x00, 0x02, 0x7C, 0x00, 0x0F, 0x80, 0x00, 0x84, 0x10, 0x00, // Row 28
    0x48, 0x00, 0x49, 0x20, 0x01, 0xE7, 0x80, 0x04, 0x9E, 0x00, 0x1E, 0x80, 0x00, 0x82, 0x20, 0x00, // Row 29
    0x24, 0x00, 0x90, 0x90, 0x03, 0xC3, 0xC0, 0x09, 0x0F, 0x00, 0x3C, 0x80, 0x00, 0x81, 0x40, 0x00, // Row 30
    0x12, 0x01, 0x20, 0x48, 0x07, 0x81, 0xE0, 0x12, 0x07, 0x80, 0x78, 0x80, 0x01, 0x00, 0x80, 0x00, // Row 31
    0x09, 0x02, 0x40, 0x24, 0x0F, 0x00, 0xF0, 0x24, 0x03, 0xC0, 0xF0, 0xFF, 0xFF, 0x00, 0x00, 0x00, // Row 32
    0x06, 0x85, 0x80, 0x1A, 0x1E, 0x00, 0x78, 0x58, 0x01, 0xE1, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 33
};

static const struct sprite_slice SYMBOL_SLICES[SYMBOL_COUNT] = {
    [SYMBOL_USB]                = { .x = 104, .y = 0,  .width = 12, .height = 10 },
    [SYMBOL_BLUETOOTH]          = { .x = 118, .y = 0,  .width = 8,  .height = 8  },
    [SYMBOL_BATTERY_FRAME]      = { .x = 88,  .y = 23, .width = 17, .height = 10 },
    [SYMBOL_SPLIT_CONNECTED]    = { .x = 104, .y = 11, .width = 13, .height = 9  },
    [SYMBOL_SPLIT_DISCONNECTED] = { .x = 106, .y = 23, .width = 13, .height = 9  },
    [SYMBOL_ARROW_HEAD]         = { .x = 118, .y = 11, .width = 3,  .height = 5  },
    [SYMBOL_ARROW_DOT]          = { .x = 122, .y = 11, .width = 3,  .height = 5  },
    [SYMBOL_BRACKET_LAYER_0]    = { .x = 0,   .y = 23, .width = 22, .height = 11 },
    [SYMBOL_BRACKET_LAYER_1]    = { .x = 22,  .y = 23, .width = 22, .height = 11 },
    [SYMBOL_BRACKET_LAYER_2]    = { .x = 44,  .y = 23, .width = 22, .height = 11 },
    [SYMBOL_BRACKET_LAYER_3]    = { .x = 66,  .y = 23, .width = 22, .height = 11 },
    [SYMBOL_SKULL_LAYER_0]      = { .x = 0,   .y = 0,  .width = 26, .height = 23 },
    [SYMBOL_SKULL_LAYER_1]      = { .x = 26,  .y = 0,  .width = 26, .height = 23 },
    [SYMBOL_SKULL_LAYER_2]      = { .x = 52,  .y = 0,  .width = 26, .height = 23 },
    [SYMBOL_SKULL_LAYER_3]      = { .x = 78,  .y = 0,  .width = 26, .height = 23 },
};

/* Font & Character Atlas structures */
struct font_glyph {
    uint16_t codepoint;
    uint8_t x;
    uint8_t y;
    uint8_t width;
    uint8_t height;
    uint8_t advance_x;
};

struct display_font {
    const uint8_t *atlas;
    uint16_t atlas_width;
    uint16_t atlas_height;
    uint16_t atlas_stride;
    const struct font_glyph *glyphs;
    uint16_t glyph_count;
    uint8_t line_height;
    uint8_t space_advance;
};

/* Spritesheet 2: Font & Character Atlas (128x22, 1bpp, 16 bytes stride) */
#define FONT_ATLAS_WIDTH  128
#define FONT_ATLAS_HEIGHT 22
#define FONT_ATLAS_STRIDE 16

static const uint8_t FONT_ATLAS[22 * 16] = {
    0x3C, 0x18, 0x3C, 0x3C, 0x0C, 0xFF, 0x3C, 0xFC, 0x3C, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 0
    0x3C, 0x18, 0x3C, 0x3C, 0x0C, 0xFF, 0x3C, 0xFC, 0x3C, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 1
    0xC3, 0x78, 0xC3, 0xC3, 0x3C, 0xC0, 0xC0, 0x03, 0xC3, 0xC3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 2
    0xC3, 0x78, 0xC3, 0xC3, 0x3C, 0xC0, 0xC0, 0x03, 0xC3, 0xC3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 3
    0xC3, 0x18, 0x0C, 0x0C, 0xCC, 0xFC, 0xFC, 0x03, 0x3C, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 4
    0xC3, 0x18, 0x0C, 0x0C, 0xCC, 0xFC, 0xFC, 0x03, 0x3C, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 5
    0xC3, 0x18, 0x30, 0xC3, 0xFF, 0x03, 0xC3, 0x0C, 0xC3, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 6
    0xC3, 0x18, 0x30, 0xC3, 0xFF, 0x03, 0xC3, 0x0C, 0xC3, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 7
    0x3C, 0x7E, 0xFF, 0x3C, 0x0C, 0xFC, 0x3C, 0x0C, 0x3C, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 8
    0x3C, 0x7E, 0xFF, 0x3C, 0x0C, 0xFC, 0x3C, 0x0C, 0x3C, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 9
    0x6E, 0x6E, 0xFF, 0x69, 0xE7, 0x31, 0x19, 0x6E, 0x6E, 0x7F, 0xCC, 0x63, 0x31, 0xF0, 0x00, 0x00, // Row 10
    0x99, 0x99, 0x88, 0x89, 0x43, 0x51, 0xBD, 0x99, 0x99, 0x82, 0x4C, 0x63, 0x31, 0x10, 0x00, 0x00, // Row 11
    0xFE, 0x89, 0xEE, 0xBF, 0x43, 0x91, 0x5B, 0x9E, 0x9E, 0x62, 0x4A, 0xAA, 0xCA, 0x60, 0x00, 0x00, // Row 12
    0x99, 0x99, 0x88, 0x99, 0x53, 0x51, 0x19, 0x98, 0xAA, 0x12, 0x4A, 0xAB, 0x24, 0x80, 0x00, 0x00, // Row 13
    0x9E, 0x6E, 0xF8, 0x69, 0xED, 0x3F, 0x19, 0x68, 0x59, 0xE2, 0x31, 0x15, 0x24, 0xF0, 0x00, 0x00, // Row 14
    0x24, 0x6A, 0x26, 0x24, 0xD4, 0x4C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 15
    0x66, 0x66, 0xFF, 0xEC, 0xCD, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 16
    0x99, 0x99, 0x88, 0x53, 0x33, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 17
    0xFF, 0xFF, 0xEE, 0x53, 0x33, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 18
    0x99, 0x99, 0x88, 0x53, 0x33, 0x2C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 19
    0x99, 0x99, 0xFF, 0xEC, 0xCC, 0xCC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 20
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Row 21
};

static const struct font_glyph FONT_GLYPHS_DIGITS[10] = {
    { .codepoint = '0' + 0, .x = 0, .y = 0, .width = 8, .height = 10, .advance_x = 10 }, // Digit 0
    { .codepoint = '0' + 1, .x = 8, .y = 0, .width = 8, .height = 10, .advance_x = 10 }, // Digit 1
    { .codepoint = '0' + 2, .x = 16, .y = 0, .width = 8, .height = 10, .advance_x = 10 }, // Digit 2
    { .codepoint = '0' + 3, .x = 24, .y = 0, .width = 8, .height = 10, .advance_x = 10 }, // Digit 3
    { .codepoint = '0' + 4, .x = 32, .y = 0, .width = 8, .height = 10, .advance_x = 10 }, // Digit 4
    { .codepoint = '0' + 5, .x = 40, .y = 0, .width = 8, .height = 10, .advance_x = 10 }, // Digit 5
    { .codepoint = '0' + 6, .x = 48, .y = 0, .width = 8, .height = 10, .advance_x = 10 }, // Digit 6
    { .codepoint = '0' + 7, .x = 56, .y = 0, .width = 8, .height = 10, .advance_x = 10 }, // Digit 7
    { .codepoint = '0' + 8, .x = 64, .y = 0, .width = 8, .height = 10, .advance_x = 10 }, // Digit 8
    { .codepoint = '0' + 9, .x = 72, .y = 0, .width = 8, .height = 10, .advance_x = 10 }, // Digit 9
};

static const struct font_glyph FONT_GLYPHS_TEXT[38] = {
    { .codepoint = 'A', .x = 0, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter A
    { .codepoint = 'B', .x = 4, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter B
    { .codepoint = 'C', .x = 8, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter C
    { .codepoint = 'D', .x = 12, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter D
    { .codepoint = 'E', .x = 16, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter E
    { .codepoint = 'F', .x = 20, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter F
    { .codepoint = 'G', .x = 24, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter G
    { .codepoint = 'H', .x = 28, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter H
    { .codepoint = 'I', .x = 32, .y = 10, .width = 3, .height = 5, .advance_x = 4 }, // Letter I
    { .codepoint = 'J', .x = 35, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter J
    { .codepoint = 'K', .x = 39, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter K
    { .codepoint = 'L', .x = 43, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter L
    { .codepoint = 'M', .x = 47, .y = 10, .width = 5, .height = 5, .advance_x = 6 }, // Letter M
    { .codepoint = 'N', .x = 52, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter N
    { .codepoint = 'O', .x = 56, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter O
    { .codepoint = 'P', .x = 60, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter P
    { .codepoint = 'Q', .x = 64, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter Q
    { .codepoint = 'R', .x = 68, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter R
    { .codepoint = 'S', .x = 72, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter S
    { .codepoint = 'T', .x = 76, .y = 10, .width = 5, .height = 5, .advance_x = 6 }, // Letter T
    { .codepoint = 'U', .x = 81, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter U
    { .codepoint = 'V', .x = 85, .y = 10, .width = 5, .height = 5, .advance_x = 6 }, // Letter V
    { .codepoint = 'W', .x = 90, .y = 10, .width = 5, .height = 5, .advance_x = 6 }, // Letter W
    { .codepoint = 'X', .x = 95, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter X
    { .codepoint = 'Y', .x = 99, .y = 10, .width = 5, .height = 5, .advance_x = 6 }, // Letter Y
    { .codepoint = 'Z', .x = 104, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter Z
    { .codepoint = 0x00C1, .x = 0, .y = 15, .width = 4, .height = 6, .advance_x = 5 }, // Letter Á
    { .codepoint = 0x00C0, .x = 4, .y = 15, .width = 4, .height = 6, .advance_x = 5 }, // Letter À
    { .codepoint = 0x00C2, .x = 8, .y = 15, .width = 4, .height = 6, .advance_x = 5 }, // Letter Â
    { .codepoint = 0x00C3, .x = 12, .y = 15, .width = 4, .height = 6, .advance_x = 5 }, // Letter Ã
    { .codepoint = 0x00C9, .x = 16, .y = 15, .width = 4, .height = 6, .advance_x = 5 }, // Letter É
    { .codepoint = 0x00CA, .x = 20, .y = 15, .width = 4, .height = 6, .advance_x = 5 }, // Letter Ê
    { .codepoint = 0x00CD, .x = 24, .y = 15, .width = 3, .height = 6, .advance_x = 4 }, // Letter Í
    { .codepoint = 0x00D3, .x = 27, .y = 15, .width = 4, .height = 6, .advance_x = 5 }, // Letter Ó
    { .codepoint = 0x00D4, .x = 31, .y = 15, .width = 4, .height = 6, .advance_x = 5 }, // Letter Ô
    { .codepoint = 0x00D5, .x = 35, .y = 15, .width = 4, .height = 6, .advance_x = 5 }, // Letter Õ
    { .codepoint = 0x00DA, .x = 39, .y = 15, .width = 4, .height = 6, .advance_x = 5 }, // Letter Ú
    { .codepoint = 0x00C7, .x = 43, .y = 15, .width = 4, .height = 6, .advance_x = 5 }, // Letter Ç
};

static const struct font_glyph FONT_GLYPHS_ALL[48] = {
    { .codepoint = '0' + 0, .x = 0, .y = 0, .width = 8, .height = 10, .advance_x = 10 }, // Digit 0
    { .codepoint = '0' + 1, .x = 8, .y = 0, .width = 8, .height = 10, .advance_x = 10 }, // Digit 1
    { .codepoint = '0' + 2, .x = 16, .y = 0, .width = 8, .height = 10, .advance_x = 10 }, // Digit 2
    { .codepoint = '0' + 3, .x = 24, .y = 0, .width = 8, .height = 10, .advance_x = 10 }, // Digit 3
    { .codepoint = '0' + 4, .x = 32, .y = 0, .width = 8, .height = 10, .advance_x = 10 }, // Digit 4
    { .codepoint = '0' + 5, .x = 40, .y = 0, .width = 8, .height = 10, .advance_x = 10 }, // Digit 5
    { .codepoint = '0' + 6, .x = 48, .y = 0, .width = 8, .height = 10, .advance_x = 10 }, // Digit 6
    { .codepoint = '0' + 7, .x = 56, .y = 0, .width = 8, .height = 10, .advance_x = 10 }, // Digit 7
    { .codepoint = '0' + 8, .x = 64, .y = 0, .width = 8, .height = 10, .advance_x = 10 }, // Digit 8
    { .codepoint = '0' + 9, .x = 72, .y = 0, .width = 8, .height = 10, .advance_x = 10 }, // Digit 9
    { .codepoint = 'A', .x = 0, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter A
    { .codepoint = 'B', .x = 4, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter B
    { .codepoint = 'C', .x = 8, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter C
    { .codepoint = 'D', .x = 12, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter D
    { .codepoint = 'E', .x = 16, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter E
    { .codepoint = 'F', .x = 20, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter F
    { .codepoint = 'G', .x = 24, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter G
    { .codepoint = 'H', .x = 28, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter H
    { .codepoint = 'I', .x = 32, .y = 10, .width = 3, .height = 5, .advance_x = 4 }, // Letter I
    { .codepoint = 'J', .x = 35, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter J
    { .codepoint = 'K', .x = 39, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter K
    { .codepoint = 'L', .x = 43, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter L
    { .codepoint = 'M', .x = 47, .y = 10, .width = 5, .height = 5, .advance_x = 6 }, // Letter M
    { .codepoint = 'N', .x = 52, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter N
    { .codepoint = 'O', .x = 56, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter O
    { .codepoint = 'P', .x = 60, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter P
    { .codepoint = 'Q', .x = 64, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter Q
    { .codepoint = 'R', .x = 68, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter R
    { .codepoint = 'S', .x = 72, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter S
    { .codepoint = 'T', .x = 76, .y = 10, .width = 5, .height = 5, .advance_x = 6 }, // Letter T
    { .codepoint = 'U', .x = 81, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter U
    { .codepoint = 'V', .x = 85, .y = 10, .width = 5, .height = 5, .advance_x = 6 }, // Letter V
    { .codepoint = 'W', .x = 90, .y = 10, .width = 5, .height = 5, .advance_x = 6 }, // Letter W
    { .codepoint = 'X', .x = 95, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter X
    { .codepoint = 'Y', .x = 99, .y = 10, .width = 5, .height = 5, .advance_x = 6 }, // Letter Y
    { .codepoint = 'Z', .x = 104, .y = 10, .width = 4, .height = 5, .advance_x = 5 }, // Letter Z
    { .codepoint = 0x00C1, .x = 0, .y = 15, .width = 4, .height = 6, .advance_x = 5 }, // Letter Á
    { .codepoint = 0x00C0, .x = 4, .y = 15, .width = 4, .height = 6, .advance_x = 5 }, // Letter À
    { .codepoint = 0x00C2, .x = 8, .y = 15, .width = 4, .height = 6, .advance_x = 5 }, // Letter Â
    { .codepoint = 0x00C3, .x = 12, .y = 15, .width = 4, .height = 6, .advance_x = 5 }, // Letter Ã
    { .codepoint = 0x00C9, .x = 16, .y = 15, .width = 4, .height = 6, .advance_x = 5 }, // Letter É
    { .codepoint = 0x00CA, .x = 20, .y = 15, .width = 4, .height = 6, .advance_x = 5 }, // Letter Ê
    { .codepoint = 0x00CD, .x = 24, .y = 15, .width = 3, .height = 6, .advance_x = 4 }, // Letter Í
    { .codepoint = 0x00D3, .x = 27, .y = 15, .width = 4, .height = 6, .advance_x = 5 }, // Letter Ó
    { .codepoint = 0x00D4, .x = 31, .y = 15, .width = 4, .height = 6, .advance_x = 5 }, // Letter Ô
    { .codepoint = 0x00D5, .x = 35, .y = 15, .width = 4, .height = 6, .advance_x = 5 }, // Letter Õ
    { .codepoint = 0x00DA, .x = 39, .y = 15, .width = 4, .height = 6, .advance_x = 5 }, // Letter Ú
    { .codepoint = 0x00C7, .x = 43, .y = 15, .width = 4, .height = 6, .advance_x = 5 }, // Letter Ç
};

static const struct display_font font_digits = {
    .atlas = FONT_ATLAS,
    .atlas_width = FONT_ATLAS_WIDTH,
    .atlas_height = FONT_ATLAS_HEIGHT,
    .atlas_stride = FONT_ATLAS_STRIDE,
    .glyphs = FONT_GLYPHS_DIGITS,
    .glyph_count = 10,
    .line_height = 10,
    .space_advance = 6,
};

static const struct display_font font_text = {
    .atlas = FONT_ATLAS,
    .atlas_width = FONT_ATLAS_WIDTH,
    .atlas_height = FONT_ATLAS_HEIGHT,
    .atlas_stride = FONT_ATLAS_STRIDE,
    .glyphs = FONT_GLYPHS_TEXT,
    .glyph_count = sizeof(FONT_GLYPHS_TEXT) / sizeof(FONT_GLYPHS_TEXT[0]),
    .line_height = 5,
    .space_advance = 3,
};

static const struct display_font font_default = {
    .atlas = FONT_ATLAS,
    .atlas_width = FONT_ATLAS_WIDTH,
    .atlas_height = FONT_ATLAS_HEIGHT,
    .atlas_stride = FONT_ATLAS_STRIDE,
    .glyphs = FONT_GLYPHS_ALL,
    .glyph_count = sizeof(FONT_GLYPHS_ALL) / sizeof(FONT_GLYPHS_ALL[0]),
    .line_height = 10,
    .space_advance = 3,
};

/* Interactive Screen Layout & Widget Architecture */
#define HAS_CUSTOM_LAYOUT_BLOCKS 1

enum display_widget_type {
    WIDGET_TYPE_NONE = 0,
    WIDGET_TYPE_OUTPUT_STATUS,
    WIDGET_TYPE_BATTERY,
    WIDGET_TYPE_LAYER,
    WIDGET_TYPE_WPM,
    WIDGET_TYPE_WPM_CHART,
    WIDGET_TYPE_BRANDING,
    WIDGET_TYPE_SPLIT,
    WIDGET_TYPE_SCREENSAVER,
    WIDGET_TYPE_CAPS_LOCK,
};

struct display_layout_block {
    uint8_t type;
    int16_t x;
    int16_t y;
    uint8_t width;
    uint8_t height;
    bool enabled;
    uint8_t mode;
    int16_t param1;
    int16_t param2;
    const char *custom_text;
    uint16_t symbol_id;
};

static const struct display_layout_block LAYOUT_LEFT_ACTIVE_BLOCKS[6] = {
    { .type = WIDGET_TYPE_OUTPUT_STATUS, .x = 0, .y = 0, .width = 12, .height = 10, .enabled = true, .mode = 0, .param1 = 0, .param2 = 0, .custom_text = NULL, .symbol_id = SYMBOL_USB },
    { .type = WIDGET_TYPE_BATTERY, .x = 13, .y = 3, .width = 17, .height = 10, .enabled = true, .mode = 0, .param1 = 0, .param2 = 0, .custom_text = NULL, .symbol_id = SYMBOL_BATTERY_FRAME },
    { .type = WIDGET_TYPE_LAYER, .x = 5, .y = 25, .width = 24, .height = 12, .enabled = true, .mode = 0, .param1 = 0, .param2 = 0, .custom_text = NULL, .symbol_id = SYMBOL_BRACKET_LAYER_0 },
    { .type = WIDGET_TYPE_SCREENSAVER, .x = 3, .y = 47, .width = 26, .height = 23, .enabled = true, .mode = 0, .param1 = 0, .param2 = 0, .custom_text = NULL, .symbol_id = SYMBOL_SKULL_LAYER_0 },
    { .type = WIDGET_TYPE_WPM, .x = 2, .y = 83, .width = 28, .height = 18, .enabled = true, .mode = 0, .param1 = 0, .param2 = 0, .custom_text = NULL, .symbol_id = SYMBOL_ARROW_HEAD },
    { .type = WIDGET_TYPE_SPLIT, .x = 10, .y = 116, .width = 13, .height = 9, .enabled = true, .mode = 0, .param1 = 0, .param2 = 0, .custom_text = NULL, .symbol_id = SYMBOL_SPLIT_CONNECTED },
};
#define LAYOUT_LEFT_ACTIVE_COUNT 6

static const struct display_layout_block LAYOUT_LEFT_IDLE_BLOCKS[3] = {
    { .type = WIDGET_TYPE_SCREENSAVER, .x = 3, .y = 47, .width = 26, .height = 23, .enabled = true, .mode = 0, .param1 = 0, .param2 = 0, .custom_text = NULL, .symbol_id = SYMBOL_SKULL_LAYER_0 },
    { .type = WIDGET_TYPE_BRANDING, .x = 3, .y = 73, .width = 26, .height = 5, .enabled = true, .mode = 1, .param1 = 0, .param2 = 0, .custom_text = NULL, .symbol_id = 0 },
    { .type = WIDGET_TYPE_SPLIT, .x = 10, .y = 116, .width = 13, .height = 9, .enabled = true, .mode = 0, .param1 = 0, .param2 = 0, .custom_text = NULL, .symbol_id = SYMBOL_SPLIT_CONNECTED },
};
#define LAYOUT_LEFT_IDLE_COUNT 3

static const struct display_layout_block LAYOUT_RIGHT_ACTIVE_BLOCKS[2] = {
    { .type = WIDGET_TYPE_BATTERY, .x = 7, .y = 3, .width = 17, .height = 10, .enabled = true, .mode = 0, .param1 = 0, .param2 = 0, .custom_text = NULL, .symbol_id = SYMBOL_BATTERY_FRAME },
    { .type = WIDGET_TYPE_SPLIT, .x = 10, .y = 116, .width = 13, .height = 9, .enabled = true, .mode = 0, .param1 = 0, .param2 = 0, .custom_text = NULL, .symbol_id = SYMBOL_SPLIT_CONNECTED },
};
#define LAYOUT_RIGHT_ACTIVE_COUNT 2

static const struct display_layout_block LAYOUT_RIGHT_IDLE_BLOCKS[1] = {
    { .type = WIDGET_TYPE_SPLIT, .x = 10, .y = 116, .width = 13, .height = 9, .enabled = true, .mode = 0, .param1 = 0, .param2 = 0, .custom_text = NULL, .symbol_id = SYMBOL_SPLIT_CONNECTED },
};
#define LAYOUT_RIGHT_IDLE_COUNT 1

/* ZMK_DISPLAY_STUDIO_METADATA
{
  "version": 1,
  "screenDimensions": {
    "width": 32,
    "height": 128
  },
  "leftBlocks": [
    { "id": "left-output", "widgetType": "connection", "instanceId": "inst_left_output", "name": "Output Status", "x": 0, "y": 0, "width": 12, "height": 10, "enabled": true, "side": "left" },
    { "id": "left-battery", "widgetType": "battery", "instanceId": "inst_left_battery", "name": "Battery Meter", "x": 13, "y": 3, "width": 17, "height": 10, "enabled": true, "side": "left" },
    { "id": "left-layer", "widgetType": "layer-banner", "instanceId": "inst_left_layer", "name": "Layer Banner", "x": 5, "y": 25, "width": 24, "height": 12, "enabled": true, "side": "left" },
    { "id": "left-screensaver", "widgetType": "screensaver", "instanceId": "inst_left_screensaver", "name": "Mascot Image", "x": 3, "y": 47, "width": 26, "height": 23, "enabled": true, "side": "left" },
    { "id": "left-wpm", "widgetType": "wpm", "instanceId": "inst_left_wpm", "name": "WPM Gauge", "x": 2, "y": 83, "width": 28, "height": 18, "enabled": true, "side": "left" },
    { "id": "left-split", "widgetType": "split", "instanceId": "inst_left_split", "name": "Split Link", "x": 10, "y": 116, "width": 13, "height": 9, "enabled": true, "side": "left" }
  ],
  "idleLeftBlocks": [
    { "id": "idle-left-screensaver", "widgetType": "screensaver", "instanceId": "inst_idle_left_screensaver", "name": "Mascot Image", "x": 3, "y": 47, "width": 26, "height": 23, "enabled": true, "side": "left" },
    { "id": "idle-left-branding", "widgetType": "branding", "instanceId": "inst_idle_left_branding", "name": "SCYAN", "x": 3, "y": 73, "width": 26, "height": 5, "enabled": true, "side": "left" },
    { "id": "idle-left-split", "widgetType": "split", "instanceId": "inst_idle_left_split", "name": "Split Link", "x": 10, "y": 116, "width": 13, "height": 9, "enabled": true, "side": "left" }
  ],
  "rightBlocks": [
    { "id": "right-battery", "widgetType": "battery", "instanceId": "inst_right_battery", "name": "Battery Meter", "x": 7, "y": 3, "width": 17, "height": 10, "enabled": true, "side": "right" },
    { "id": "right-split", "widgetType": "split", "instanceId": "inst_right_split", "name": "Split Link", "x": 10, "y": 116, "width": 13, "height": 9, "enabled": true, "side": "right" }
  ],
  "idleRightBlocks": [
    { "id": "idle-right-split", "widgetType": "split", "instanceId": "inst_idle_right_split", "name": "Split Link", "x": 10, "y": 116, "width": 13, "height": 9, "enabled": true, "side": "right" }
  ],
  "widgetInstances": {
    "connection": [{ "id": "inst_left_output", "widgetTypeId": "connection", "label": "Output Status", "config": { "mode": "symbol" } }],
    "battery": [{ "id": "inst_left_battery", "widgetTypeId": "battery", "label": "Battery Meter", "config": { "mode": "symbol" } }],
    "layer-banner": [{ "id": "inst_left_layer", "widgetTypeId": "layer-banner", "label": "Layer Banner", "config": { "mode": "symbol" } }],
    "screensaver": [{ "id": "inst_left_screensaver", "widgetTypeId": "screensaver", "label": "Mascot Image", "config": { "mode": "symbol", "groupId": "SYMBOL_SKULL_LAYER_0" } }],
    "wpm": [{ "id": "inst_left_wpm", "widgetTypeId": "wpm", "label": "WPM Gauge", "config": { "mode": "symbol" } }],
    "branding": [{ "id": "inst_idle_left_branding", "widgetTypeId": "branding", "label": "User Branding", "config": { "mode": "font", "textEntries": ["SCYAN"] } }],
    "split": [{ "id": "inst_left_split", "widgetTypeId": "split", "label": "Split Link", "config": { "mode": "symbol" } }]
  }
}
*/
