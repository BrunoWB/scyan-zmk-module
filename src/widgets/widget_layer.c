/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "widgets.h"
#include "canvas.h"
#include "font_renderer.h"

void widget_render_layer(const struct display_layout_block *b, const struct custom_status_state *state) {
    if (!b || !b->enabled) return;

    uint8_t layer = state->active_layer;

    if (b->mode == 1) {
        // Font / Text mode
        if (b->text_count > 0 && layer < b->text_count && b->text_entries[layer]) {
            font_draw_text(b->x, b->y, font_get_small(), b->text_entries[layer]);
        } else if (state->layer_name[0] != '\0') {
            font_draw_text(b->x, b->y, font_get_small(), state->layer_name);
        } else if (b->custom_text && b->custom_text[0] != '\0') {
            font_draw_text(b->x, b->y, font_get_small(), b->custom_text);
        } else {
            char buf[16];
            snprintf(buf, sizeof(buf), "L%d", layer);
            font_draw_text(b->x, b->y, font_get_small(), buf);
        }
    } else {
        // Symbol / Icon mode
        if (b->symbol_count > 0) {
            uint8_t idx = layer % b->symbol_count;
            canvas_draw_symbol(b->x, b->y, b->symbol_ids[idx]);
        } else if (b->symbol_id != 0) {
            canvas_draw_symbol(b->x, b->y, b->symbol_id);
        } else {
            char buf[16];
            snprintf(buf, sizeof(buf), "L%d", layer);
            font_draw_text(b->x, b->y, font_get_small(), buf);
        }
    }
}
