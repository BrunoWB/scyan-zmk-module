/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#include "widgets.h"
#include "canvas.h"
#include "font_renderer.h"

void widget_render_loop(const struct display_layout_block *b, const struct custom_status_state *state) {
    if (!b || !b->enabled) return;

    if (b->symbol_count > 0) {
        uint8_t idx = 0;
        if (state) {
            bool no_loop = (b->param2 == 1);
            if (no_loop) {
                idx = (state->loop_tick < b->symbol_count) ? (uint8_t)state->loop_tick : (b->symbol_count - 1);
            } else {
                idx = (uint8_t)(state->loop_tick % b->symbol_count);
            }
        }
        uint16_t sym = (idx < MAX_BLOCK_SYMBOLS && b->symbol_ids[idx] != 0)
                     ? b->symbol_ids[idx]
                     : (b->symbol_id + idx);
        canvas_draw_symbol(b->x, b->y, sym);
    } else if (b->symbol_id != 0) {
        canvas_draw_symbol(b->x, b->y, b->symbol_id);
    } else {
        const char *txt = (b->text_count > 0 && b->text_entries[0]) ? b->text_entries[0] :
                          (b->custom_text ? b->custom_text : "ANIM");
        font_draw_text(b->x, b->y, font_get_small(), txt);
    }
}
