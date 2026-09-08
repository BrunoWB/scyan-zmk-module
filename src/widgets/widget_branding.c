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
        int draw_x;

        if (b->x == 0 && b->width >= DISPLAY_VIRTUAL_WIDTH) {
            // Full screen width centering intent
            draw_x = (DISPLAY_VIRTUAL_WIDTH - text_w) / 2;
        } else if (b->width > text_w && b->width < DISPLAY_VIRTUAL_WIDTH) {
            // Centered within bounded block
            draw_x = b->x + (b->width - text_w) / 2;
        } else {
            draw_x = b->x;
        }

        // Hardware display boundary safety clamp: prevent clipping of trailing characters
        if (draw_x + text_w > DISPLAY_VIRTUAL_WIDTH) {
            draw_x = DISPLAY_VIRTUAL_WIDTH - text_w;
        }
        if (draw_x < 0) {
            draw_x = 0;
        }

        font_draw_text(draw_x, b->y, font, text);
    }
}
