/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#include "widgets.h"
#include "canvas.h"
#include "font_renderer.h"

void widget_render_screensaver(const struct display_layout_block *b, const struct custom_status_state *state) {
    if (!b || !b->enabled) return;
    (void)state;

    if (b->symbol_count > 0) {
        // Render static screensaver symbol
        canvas_draw_symbol(b->x, b->y, b->symbol_ids[0]);
    } else if (b->symbol_id != 0) {
        canvas_draw_symbol(b->x, b->y, b->symbol_id);
    } else if (b->custom_text && b->custom_text[0] != '\0') {
        font_draw_text(b->x, b->y, font_get_small(), b->custom_text);
    }
}
