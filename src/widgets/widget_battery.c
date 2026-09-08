/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "widgets.h"
#include "canvas.h"
#include "font_renderer.h"

void widget_render_battery(const struct display_layout_block *b, const struct custom_status_state *state) {
    if (!b || !b->enabled) return;

    uint8_t level = state->battery_level;
    bool charging = state->charging;

    if (b->mode == 1) {
        // Font / Text mode
        if (b->text_count > 0) {
            int divs = b->text_count;
            int idx = ((int)level * divs) / 100;
            if (idx >= divs) idx = divs - 1;
            if (idx < 0) idx = 0;
            if (b->text_entries[idx]) {
                font_draw_text(b->x, b->y, font_get_small(), b->text_entries[idx]);
            }
        } else {
            char buf[12];
            if (charging) {
                snprintf(buf, sizeof(buf), "+%d%%", level);
            } else {
                snprintf(buf, sizeof(buf), "%d%%", level);
            }
            font_draw_text(b->x, b->y, font_get_small(), buf);
        }
    } else {
        // Symbol / Icon mode
        if (b->symbol_count > 0) {
            int steps = (b->param1 > 1) ? b->param1 : b->symbol_count;
            if (steps > b->symbol_count) steps = b->symbol_count;

            int idx = ((int)level * (steps - 1)) / 100;
            if (idx >= steps) idx = steps - 1;
            if (idx < 0) idx = 0;

            canvas_draw_symbol(b->x, b->y, b->symbol_ids[idx]);
        } else {
            // Built-in geometric battery icon fallback (16x10)
            int bx = b->x;
            int by = b->y;

            // Frame
            canvas_fill_rect(bx, by, 15, 1, 1);
            canvas_fill_rect(bx, by + 8, 15, 1, 1);
            canvas_fill_rect(bx, by, 1, 9, 1);
            canvas_fill_rect(bx + 14, by, 1, 9, 1);
            // Nipple
            canvas_fill_rect(bx + 15, by + 2, 2, 5, 1);

            // Fill bars (max 11 width)
            int bar_w = ((int)level * 11) / 100;
            if (bar_w > 0) {
                canvas_fill_rect(bx + 2, by + 2, bar_w, 5, 1);
            }

            if (charging) {
                // Lightning bolt cutout/mark in middle
                canvas_fill_rect(bx + 6, by + 1, 3, 7, 1);
            }
        }
    }
}

