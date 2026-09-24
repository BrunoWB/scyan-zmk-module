/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#include "widgets.h"

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
    case WIDGET_TYPE_BONGO:
        widget_render_bongo(b, state);
        break;
    case WIDGET_TYPE_LOOP:
        widget_render_loop(b, state);
        break;
    case WIDGET_TYPE_TYPEWRITER:
        widget_render_typewriter(b, state);
        break;
    case WIDGET_TYPE_KEYPRESS:
        widget_render_keypress(b, state);
        break;
    default:
        break;
    }
}
