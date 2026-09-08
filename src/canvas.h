/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "scyan/types.h"

void canvas_clear(void);
void canvas_set_pixel(int x, int y, uint8_t val);
uint8_t canvas_get_pixel(int x, int y);
void canvas_fill_rect(int x, int y, int w, int h, uint8_t val);
void canvas_draw_bitmap_1bpp(int dst_x, int dst_y, int w, int h, const uint8_t *bmp, int stride);
void canvas_draw_atlas_subrect(int dst_x, int dst_y, const uint8_t *atlas, int stride, int sx, int sy, int sw, int sh);
void canvas_draw_slice(int dst_x, int dst_y, const struct sprite_slice *slice);
void canvas_draw_symbol(int dst_x, int dst_y, uint16_t symbol_id);
const uint8_t (*canvas_get_vbuf(void))[DISPLAY_VIRTUAL_WIDTH];

