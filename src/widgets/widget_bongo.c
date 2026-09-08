/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#include "widgets.h"
#include "canvas.h"
#include "font_renderer.h"

void widget_render_bongo(const struct display_layout_block *b, const struct custom_status_state *state) {
    if (!b || !b->enabled) return;

    if (b->mode == 0 && b->symbol_count >= 3) {
        // Symbol mode with 3 slices:
        // Index 0: Neutral / Idle pose
        // Index 1: Left arm tap
        // Index 2: Right arm tap
        uint8_t idx = state->bongo_state;
        if (idx >= b->symbol_count) {
            idx = 0;
        }
        canvas_draw_symbol(b->x, b->y, b->symbol_ids[idx]);
    } else if (b->mode == 0 && b->symbol_count > 0) {
        canvas_draw_symbol(b->x, b->y, b->symbol_ids[0]);
    } else if (b->mode == 0 && b->symbol_id) {
        canvas_draw_symbol(b->x, b->y, b->symbol_id);
    } else {
        // Fallback text mode
        const char *txt = (b->text_count > 0 && b->text_entries[0]) ? b->text_entries[0] :
                          (b->custom_text ? b->custom_text : "(=^.^=)");
        font_draw_text(b->x, b->y, font_get_small(), txt);
    }
}
