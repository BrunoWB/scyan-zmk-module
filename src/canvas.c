/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include "canvas.h"

static uint8_t vbuf[DISPLAY_VIRTUAL_HEIGHT][DISPLAY_VIRTUAL_WIDTH];

void canvas_clear(void) {
    memset(vbuf, 0, sizeof(vbuf));
}

void canvas_set_pixel(int x, int y, uint8_t val) {
    if (x >= 0 && x < DISPLAY_VIRTUAL_WIDTH && y >= 0 && y < DISPLAY_VIRTUAL_HEIGHT) {
        vbuf[y][x] = val ? 1 : 0;
    }
}

uint8_t canvas_get_pixel(int x, int y) {
    if (x >= 0 && x < DISPLAY_VIRTUAL_WIDTH && y >= 0 && y < DISPLAY_VIRTUAL_HEIGHT) {
        return vbuf[y][x];
    }
    return 0;
}

void canvas_fill_rect(int x, int y, int w, int h, uint8_t val) {
    int x0 = x < 0 ? 0 : x;
    int y0 = y < 0 ? 0 : y;
    int x1 = (x + w) > DISPLAY_VIRTUAL_WIDTH ? DISPLAY_VIRTUAL_WIDTH : (x + w);
    int y1 = (y + h) > DISPLAY_VIRTUAL_HEIGHT ? DISPLAY_VIRTUAL_HEIGHT : (y + h);

    for (int r = y0; r < y1; r++) {
        for (int c = x0; c < x1; c++) {
            vbuf[r][c] = val ? 1 : 0;
        }
    }
}

void canvas_draw_bitmap_1bpp(int dst_x, int dst_y, int w, int h, const uint8_t *bmp, int stride) {
    if (!bmp) return;
    for (int r = 0; r < h; r++) {
        int py = dst_y + r;
        if (py < 0 || py >= DISPLAY_VIRTUAL_HEIGHT) continue;
        for (int c = 0; c < w; c++) {
            int px = dst_x + c;
            if (px < 0 || px >= DISPLAY_VIRTUAL_WIDTH) continue;
            int byte_idx = r * stride + (c / 8);
            int bit_idx = 7 - (c % 8);
            if ((bmp[byte_idx] >> bit_idx) & 1) {
                vbuf[py][px] = 1;
            }
        }
    }
}

void canvas_draw_atlas_subrect(int dst_x, int dst_y, const uint8_t *atlas, int stride, int sx, int sy, int sw, int sh) {
    if (!atlas) return;
    for (int r = 0; r < sh; r++) {
        int py = dst_y + r;
        if (py < 0 || py >= DISPLAY_VIRTUAL_HEIGHT) continue;
        for (int c = 0; c < sw; c++) {
            int px = dst_x + c;
            if (px < 0 || px >= DISPLAY_VIRTUAL_WIDTH) continue;
            int byte_idx = (sy + r) * stride + ((sx + c) / 8);
            int bit_idx = 7 - ((sx + c) % 8);
            if ((atlas[byte_idx] >> bit_idx) & 1) {
                vbuf[py][px] = 1;
            }
        }
    }
}

void canvas_draw_slice(int dst_x, int dst_y, const struct sprite_slice *slice) {
    if (!slice || slice->width == 0 || slice->height == 0) return;
    canvas_draw_atlas_subrect(dst_x, dst_y, SYMBOLS_ATLAS, SYMBOLS_ATLAS_STRIDE,
                             slice->x, slice->y, slice->width, slice->height);
}

void canvas_draw_symbol(int dst_x, int dst_y, uint16_t symbol_id) {
    if (symbol_id >= SYMBOL_COUNT) return;
    const struct sprite_slice *s = &SYMBOL_SLICES[symbol_id];
    canvas_draw_slice(dst_x, dst_y, s);
}

const uint8_t (*canvas_get_vbuf(void))[DISPLAY_VIRTUAL_WIDTH] {
    return vbuf;
}
