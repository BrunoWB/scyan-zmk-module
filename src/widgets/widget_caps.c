/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#include "widgets.h"
#include "canvas.h"
#include "font_renderer.h"

void widget_render_caps(const struct display_layout_block *b, const struct custom_status_state *state) {
    if (!b || !b->enabled) return;

    if (state->caps_lock) {
        if (b->mode == 1) {
            const char *txt = (b->text_count > 0 && b->text_entries[0]) ? b->text_entries[0] :
                              (b->custom_text ? b->custom_text : "CAPS");
            font_draw_text(b->x, b->y, font_get_small(), txt);
        } else {
            uint16_t sym = (b->symbol_count > 0) ? b->symbol_ids[0] : b->symbol_id;
            canvas_draw_symbol(b->x, b->y, sym);
        }
    } else {
        // If inactive symbol exists, render it
        if (b->mode == 0 && b->symbol_count > 1) {
            canvas_draw_symbol(b->x, b->y, b->symbol_ids[1]);
        }
    }
}
