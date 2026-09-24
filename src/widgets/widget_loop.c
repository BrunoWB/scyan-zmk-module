/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#include "widgets.h"
#include "canvas.h"
#include "font_renderer.h"

void widget_render_loop(const struct display_layout_block *b, const struct custom_status_state *state) {
    if (!b || !b->enabled) return;

    uint8_t total_frames = (b->param3 > 0) ? (uint8_t)b->param3 : b->symbol_count;
    if (total_frames > 0) {
        uint8_t idx = 0;
        if (state) {
            bool no_loop = (b->param2 == 1);
            if (no_loop) {
                idx = (state->loop_tick < total_frames) ? (uint8_t)state->loop_tick : (total_frames - 1);
            } else {
                idx = (uint8_t)(state->loop_tick % total_frames);
            }
        }
        uint16_t sym = (idx < MAX_BLOCK_SYMBOLS && idx < b->symbol_count)
                     ? b->symbol_ids[idx]
                     : (b->symbol_id + idx);
        canvas_draw_symbol(b->x, b->y, sym);
    } else {
        const char *txt = (b->text_count > 0 && b->text_entries[0]) ? b->text_entries[0] :
                          (b->custom_text ? b->custom_text : "ANIM");
        font_draw_text(b->x, b->y, font_get_small(), txt);
    }
}
