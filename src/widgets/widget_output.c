/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include "widgets.h"
#include "canvas.h"
#include "font_renderer.h"

void widget_render_output(const struct display_layout_block *b, const struct custom_status_state *state) {
    if (!b || !b->enabled) return;

    bool is_usb = (state->selected_endpoint.transport == ZMK_TRANSPORT_USB);

    if (b->mode == 1) {
        // Font / Text mode
        if (is_usb) {
            const char *txt = (b->text_count > 0 && b->text_entries[0]) ? b->text_entries[0] :
                              (b->custom_text ? b->custom_text : "USB");
            font_draw_text(b->x, b->y, font_get_small(), txt);
        } else {
            int p = state->active_profile_index;
            if (state->active_profile_connected) {
                if (b->text_count > p + 1 && b->text_entries[p + 1]) {
                    font_draw_text(b->x, b->y, font_get_small(), b->text_entries[p + 1]);
                } else {
                    char buf[16];
                    snprintf(buf, sizeof(buf), "P%d", p + 1);
                    font_draw_text(b->x, b->y, font_get_small(), buf);
                }
            } else if (state->active_profile_bonded) {
                // Reconnecting
                char buf[16];
                snprintf(buf, sizeof(buf), "P%d..", p + 1);
                font_draw_text(b->x, b->y, font_get_small(), buf);
            } else {
                // Pairing open
                char buf[16];
                snprintf(buf, sizeof(buf), "P%d+", p + 1);
                font_draw_text(b->x, b->y, font_get_small(), buf);
            }
        }
    } else {
        // Symbol / Icon mode
        if (is_usb) {
            uint16_t sym = (b->symbol_count > 0) ? b->symbol_ids[0] : b->symbol_id;
            canvas_draw_symbol(b->x, b->y, sym);
        } else {
            int p = state->active_profile_index;
            uint16_t sym;
            if (b->symbol_count >= 13) {
                // Multi-state structure:
                // 0: USB
                // 1: Disconnected
                // 2..6: Connected P1..P5
                // 7..11: Reconnect Frame 2 P1..P5
                // 12: Pairing Frame 2 (BT dots)
                if (state->active_profile_connected) {
                    sym = b->symbol_ids[2 + (p % 5)];
                } else if (state->active_profile_bonded) {
                    // Reconnecting animation: alternate 500ms between Frame 1 (Connected) and Frame 2 (Reconnect)
                    uint32_t frame = (k_uptime_get_32() / 500) % 2;
                    sym = (frame == 0) ? b->symbol_ids[2 + (p % 5)] : b->symbol_ids[7 + (p % 5)];
                } else {
                    // Pairing open animation: alternate 500ms between Frame 1 (Connected) and Frame 2 (Pairing BT dots)
                    uint32_t frame = (k_uptime_get_32() / 500) % 2;
                    sym = (frame == 0) ? b->symbol_ids[2 + (p % 5)] : b->symbol_ids[12];
                }
            } else if (b->symbol_count > p + 1) {
                sym = b->symbol_ids[p + 1];
            } else if (b->symbol_count > 1) {
                sym = b->symbol_ids[1];
            } else if (b->symbol_count > 0) {
                sym = b->symbol_ids[0];
            } else {
                sym = b->symbol_id;
            }
            canvas_draw_symbol(b->x, b->y, sym);

            // If generic symbol was used, overlay profile index
            if (b->symbol_count <= 2 && state->active_profile_connected) {
                char p_char = '1' + (p % 9);
                font_draw_char(b->x + b->width - 4, b->y + b->height - 5, font_get_small(), p_char);
            }
        }
    }
}
