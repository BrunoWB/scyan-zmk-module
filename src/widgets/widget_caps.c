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

void widget_dispatch_block(const struct display_layout_block *b, const struct custom_status_state *state) {
    if (!b || !b->enabled) return;

    switch (b->type) {
    case WIDGET_TYPE_OUTPUT_STATUS:
        widget_render_output(b, state);
        break;
    case WIDGET_TYPE_BATTERY:
        widget_render_battery(b, state);
        break;
    case WIDGET_TYPE_LAYER:
        widget_render_layer(b, state);
        break;
    case WIDGET_TYPE_WPM:
        widget_render_wpm(b, state);
        break;
    case WIDGET_TYPE_WPM_CHART:
        widget_render_wpm_chart(b, state);
        break;
    case WIDGET_TYPE_BRANDING:
        widget_render_branding(b, state);
        break;
    case WIDGET_TYPE_SPLIT:
        widget_render_split(b, state);
        break;
    case WIDGET_TYPE_SCREENSAVER:
        widget_render_screensaver(b, state);
        break;
    case WIDGET_TYPE_CAPS_LOCK:
        widget_render_caps(b, state);
        break;
    default:
        break;
    }
}

