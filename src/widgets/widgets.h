/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "scyan/types.h"

void widget_render_output(const struct display_layout_block *b, const struct custom_status_state *state);
void widget_render_battery(const struct display_layout_block *b, const struct custom_status_state *state);
void widget_render_layer(const struct display_layout_block *b, const struct custom_status_state *state);
void widget_render_wpm(const struct display_layout_block *b, const struct custom_status_state *state);
void widget_render_wpm_chart(const struct display_layout_block *b, const struct custom_status_state *state);
void widget_wpm_tick(uint8_t current_wpm);
void widget_wpm_reset_history(void);
void widget_render_branding(const struct display_layout_block *b, const struct custom_status_state *state);
void widget_render_split(const struct display_layout_block *b, const struct custom_status_state *state);
void widget_render_screensaver(const struct display_layout_block *b, const struct custom_status_state *state);
void widget_render_caps(const struct display_layout_block *b, const struct custom_status_state *state);
void widget_render_bongo(const struct display_layout_block *b, const struct custom_status_state *state);

void widget_dispatch_block(const struct display_layout_block *b, const struct custom_status_state *state);

