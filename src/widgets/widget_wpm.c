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
        if (b->text_count >= 1) {
            int idx = 0;
            if (b->text_count > 1) {
                idx = ((int)state->wpm * (b->text_count - 1)) / target;
                if (idx >= b->text_count) idx = b->text_count - 1;
                if (idx < 0) idx = 0;
            }
            if (b->text_entries[idx]) {
                const struct display_font *font = (b->param3 == 1) ? font_get_big() : font_get_small();
                font_draw_text(b->x, b->y, font, b->text_entries[idx]);
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

    if (b->mode == 1) {
        // Bar chart mode:
        // Calculate bar width based on time window:
        // Shorter time window produces wider bars; longer time window produces narrower bars.
        int gap = 1;
        int bar_w = 1;
        if (time_window <= 12) {
            bar_w = (inner_w + 4) / 8 - gap;
            if (bar_w > 5) bar_w = 5;
        } else if (time_window <= 25) {
            bar_w = (inner_w + 5) / 10 - gap;
            if (bar_w > 4) bar_w = 4;
        } else if (time_window <= 45) {
            bar_w = (inner_w + 6) / 12 - gap;
            if (bar_w > 3) bar_w = 3;
        } else if (time_window <= 75) {
            bar_w = (inner_w + 8) / 16 - gap;
            if (bar_w > 2) bar_w = 2;
        } else {
            bar_w = 1;
        }
        if (bar_w < 1) bar_w = 1;

        int step = bar_w + gap;
        int bar_idx = 0;
        while (1) {
            int bar_end = inner_x + inner_w - 1 - bar_idx * step;
            if (bar_end < inner_x) break;
            int bar_start = bar_end - bar_w + 1;
            if (bar_start < inner_x) bar_start = inner_x;

            uint8_t val;
            if (bar_idx == 0) {
                val = state->wpm;
            } else {
                int bar_center = (bar_start + bar_end) / 2;
                int age_sec = (inner_w > 1) ? ((inner_x + inner_w - 1 - bar_center) * time_window) / (inner_w - 1) : 0;
                if (age_sec == 0) {
                    val = state->wpm;
                } else if (age_sec <= wpm_history_count && (WPM_HISTORY_MAX - age_sec) >= 0) {
                    val = wpm_history[WPM_HISTORY_MAX - age_sec];
                } else {
                    val = 0;
                }
            }

            int clamped_val = val > target ? target : val;
            int bar_h = clamped_val > 0 ? (clamped_val * (inner_h - 1)) / target : 0;
            if (clamped_val > 0 && bar_h == 0) bar_h = 1;

            int py = (inner_y + inner_h - 1) - bar_h;
            int draw_w = bar_end - bar_start + 1;
            int draw_h = bar_h + 1;
            canvas_fill_rect(bar_start, py, draw_w, draw_h, 1);

            bar_idx++;
        }
    } else {
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
}
