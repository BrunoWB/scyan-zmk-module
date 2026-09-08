/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#include "widgets.h"
#include "canvas.h"
#include "font_renderer.h"

void widget_render_split(const struct display_layout_block *b, const struct custom_status_state *state) {
    if (!b || !b->enabled) return;

    bool connected = state->split_connected;

    if (b->mode == 1) {
        // Text mode
        const char *txt;
        if (connected) {
            txt = (b->text_count > 0 && b->text_entries[0]) ? b->text_entries[0] : "LINK";
        } else {
            txt = (b->text_count > 1 && b->text_entries[1]) ? b->text_entries[1] : "WAIT";
        }
        font_draw_text(b->x, b->y, font_get_small(), txt);
    } else {
        // Symbol mode
        uint16_t sym;
        if (connected) {
            sym = (b->symbol_count > 0) ? b->symbol_ids[0] : b->symbol_id;
        } else {
            sym = (b->symbol_count > 1) ? b->symbol_ids[1] : b->symbol_id;
        }
        canvas_draw_symbol(b->x, b->y, sym);
    }
}

