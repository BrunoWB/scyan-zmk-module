/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include "widgets.h"
#include "canvas.h"
#include "font_renderer.h"

#define WPM_HISTORY_MAX 64

static uint8_t wpm_history[WPM_HISTORY_MAX];
static uint8_t wpm_history_count = 0;

void widget_wpm_tick(uint8_t current_wpm) {
    for (int i = 0; i < WPM_HISTORY_MAX - 1; i++) {
        wpm_history[i] = wpm_history[i + 1];
    }
    wpm_history[WPM_HISTORY_MAX - 1] = current_wpm;
    if (wpm_history_count < WPM_HISTORY_MAX) {
        wpm_history_count++;
    }
}

void widget_wpm_reset_history(void) {
    memset(wpm_history, 0, sizeof(wpm_history));
    wpm_history_count = 0;
}

void widget_render_wpm(const struct display_layout_block *b, const struct custom_status_state *state) {
    if (!b || !b->enabled) return;

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

    int grid_size = b->param1;
    int target = b->param2 > 0 ? b->param2 : 100;
    int time_window = b->param3 > 0 ? b->param3 : 30;
    int chart_w = b->width > 0 ? b->width : 32;
    int chart_h = b->height > 0 ? b->height : 24;
    int bx = b->x;
    int by = b->y;

    // Clamp dimensions to display bounds so borders and NOW column stay on-screen
    if (bx + chart_w > DISPLAY_VIRTUAL_WIDTH) {
        chart_w = DISPLAY_VIRTUAL_WIDTH - bx;
    }
    if (by + chart_h > DISPLAY_VIRTUAL_HEIGHT) {
        chart_h = DISPLAY_VIRTUAL_HEIGHT - by;
    }
    if (chart_w <= 2 || chart_h <= 2) return;

    // Render outer border & grid points if grid_size > 0
    if (grid_size > 0) {
        canvas_fill_rect(bx, by, chart_w, 1, 1);
        canvas_fill_rect(bx, by + chart_h - 1, chart_w, 1, 1);
        canvas_fill_rect(bx, by, 1, chart_h, 1);
        canvas_fill_rect(bx + chart_w - 1, by, 1, chart_h, 1);

        for (int y = grid_size; y < chart_h - 1; y += grid_size) {
            for (int x = grid_size; x < chart_w - 1; x += grid_size) {
                canvas_set_pixel(bx + x, by + y, 1);
            }
        }
    }

    int inner_x = bx + (grid_size > 0 ? 1 : 0);
    int inner_y = by + (grid_size > 0 ? 1 : 0);
    int inner_w = chart_w - (grid_size > 0 ? 2 : 0);
    int inner_h = chart_h - (grid_size > 0 ? 2 : 0);
    if (inner_w <= 1 || inner_h <= 1) return;

    // Draw baseline if no grid/border
    if (grid_size == 0) {
        canvas_fill_rect(inner_x, inner_y + inner_h - 1, inner_w, 1, 1);
    }

    // Oscilloscope / heartbeat line:
    // Rightmost column (inner_x + inner_w - 1) is NOW (current state->wpm).
    // Older samples scroll left from rightmost column scaled across time_window.
    // Top = targetSpeed, Bottom = 0 WPM.
    int prev_py = -1;
    for (int c = 0; c < inner_w; c++) {
        int px = inner_x + c;
        uint8_t val;
        if (c == inner_w - 1) {
            val = state->wpm;
        } else {
            int age_sec = (inner_w > 1) ? ((inner_w - 1 - c) * time_window) / (inner_w - 1) : 0;
            if (age_sec == 0) {
                val = state->wpm;
            } else if (age_sec <= wpm_history_count && (WPM_HISTORY_MAX - age_sec) >= 0) {
                val = wpm_history[WPM_HISTORY_MAX - age_sec];
            } else {
                val = 0;
            }
        }

        int clamped_val = val > target ? target : val;
        int py = (inner_y + inner_h - 1) - ((clamped_val * (inner_h - 1)) / target);

        if (c > 0 && prev_py >= 0) {
            int min_y = prev_py < py ? prev_py : py;
            int max_y = prev_py > py ? prev_py : py;
            for (int y = min_y; y <= max_y; y++) {
                canvas_set_pixel(px, y, 1);
            }
        } else {
            canvas_set_pixel(px, py, 1);
        }
        prev_py = py;
    }
}
