/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#include "widgets.h"
#include "canvas.h"
#include "font_renderer.h"

void widget_render_branding(const struct display_layout_block *b, const struct custom_status_state *state) {
    if (!b || !b->enabled) return;
    (void)state;

    if (b->mode == 0 && b->symbol_count > 0) {
        // Symbol mode
        canvas_draw_symbol(b->x, b->y, b->symbol_ids[0]);
    } else {
        // Text mode (default)
        const char *text = (b->text_count > 0 && b->text_entries[0]) ? b->text_entries[0] :
                           (b->custom_text ? b->custom_text : CONFIG_CUSTOM_STATUS_SCREEN_USER_NAME);

        const struct display_font *font = font_get_small();
        int text_w = font_measure_text(font, text);
        int draw_x = (b->width > text_w) ? (b->x + (b->width - text_w) / 2) : b->x;

        font_draw_text(draw_x, b->y, font, text);
    }
}
