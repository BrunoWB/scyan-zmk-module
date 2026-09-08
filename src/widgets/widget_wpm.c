/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include "widgets.h"
#include "canvas.h"
#include "font_renderer.h"

#define WPM_HISTORY_MAX 32

static uint8_t wpm_history[WPM_HISTORY_MAX];
static uint8_t wpm_history_idx = 0;
static uint8_t wpm_history_count = 0;
static uint8_t last_recorded_wpm = 0xFF;

static void record_wpm_sample(uint8_t wpm) {
    if (wpm == last_recorded_wpm && wpm_history_count > 0) return;
    last_recorded_wpm = wpm;

    wpm_history[wpm_history_idx] = wpm;
    wpm_history_idx = (wpm_history_idx + 1) % WPM_HISTORY_MAX;
    if (wpm_history_count < WPM_HISTORY_MAX) {
        wpm_history_count++;
    }
}

void widget_render_wpm(const struct display_layout_block *b, const struct custom_status_state *state) {
    if (!b || !b->enabled) return;

    record_wpm_sample(state->wpm);
    int target = b->param2 > 0 ? b->param2 : 100;

    if (b->mode == 1) {
        // Font / Text mode
        if (b->text_count >= 2) {
            int idx = ((int)state->wpm * (b->text_count - 1)) / target;
            if (idx >= b->text_count) idx = b->text_count - 1;
            if (idx < 0) idx = 0;
            if (b->text_entries[idx]) {
                font_draw_text(b->x, b->y, font_get_small(), b->text_entries[idx]);
            }
        } else {
            char buf[16];
            snprintf(buf, sizeof(buf), "%d", state->wpm);
            font_draw_text(b->x, b->y, font_get_digits(), buf);
        }
    } else {
        // Symbol / Icon mode
        if (b->symbol_count > 0) {
            // Speedometer gauge slice
            int idx = ((int)state->wpm * b->symbol_count) / target;
            if (idx >= b->symbol_count) idx = b->symbol_count - 1;
            if (idx < 0) idx = 0;

            canvas_draw_symbol(b->x, b->y, b->symbol_ids[idx]);
        } else {
            // Text readout fallback
            char buf[8];
            snprintf(buf, sizeof(buf), "%d", state->wpm);
            font_draw_text(b->x, b->y, font_get_digits(), buf);
        }
    }
}

void widget_render_wpm_chart(const struct display_layout_block *b, const struct custom_status_state *state) {
    if (!b || !b->enabled) return;

    record_wpm_sample(state->wpm);

    int target = b->param2 > 0 ? b->param2 : 100;
    int chart_w = b->width > 0 ? b->width : 28;
    int chart_h = b->height > 0 ? b->height : 16;
    int bx = b->x;
    int by = b->y;

    // Draw baseline
    canvas_fill_rect(bx, by + chart_h - 1, chart_w, 1, 1);

    // Plot historical points from right to left
    int points = wpm_history_count < chart_w ? wpm_history_count : chart_w;
    for (int i = 0; i < points; i++) {
        int hist_pos = (wpm_history_idx - 1 - i + WPM_HISTORY_MAX) % WPM_HISTORY_MAX;
        uint8_t val = wpm_history[hist_pos];

        int bar_h = ((int)val * (chart_h - 2)) / target;
        if (bar_h > (chart_h - 2)) bar_h = chart_h - 2;
        if (bar_h < 1 && val > 0) bar_h = 1;

        int px = bx + chart_w - 1 - i;
        int py = by + chart_h - 1 - bar_h;
        canvas_fill_rect(px, py, 1, bar_h, 1);
    }
}
