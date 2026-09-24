/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "scyan/types.h"

#if IS_ENABLED(CONFIG_SCYAN_WIDGET_OUTPUT)
void widget_render_output(const struct display_layout_block *b, const struct custom_status_state *state);
#else
static inline void widget_render_output(const struct display_layout_block *b, const struct custom_status_state *state) { (void)b; (void)state; }
#endif

#if IS_ENABLED(CONFIG_SCYAN_WIDGET_BATTERY)
void widget_render_battery(const struct display_layout_block *b, const struct custom_status_state *state);
#else
static inline void widget_render_battery(const struct display_layout_block *b, const struct custom_status_state *state) { (void)b; (void)state; }
#endif

#if IS_ENABLED(CONFIG_SCYAN_WIDGET_LAYER)
void widget_render_layer(const struct display_layout_block *b, const struct custom_status_state *state);
#else
static inline void widget_render_layer(const struct display_layout_block *b, const struct custom_status_state *state) { (void)b; (void)state; }
#endif

#if IS_ENABLED(CONFIG_SCYAN_WIDGET_WPM)
void widget_render_wpm(const struct display_layout_block *b, const struct custom_status_state *state);
void widget_render_wpm_chart(const struct display_layout_block *b, const struct custom_status_state *state);
void widget_wpm_tick(uint8_t current_wpm);
void widget_wpm_reset_history(void);
#else
static inline void widget_render_wpm(const struct display_layout_block *b, const struct custom_status_state *state) { (void)b; (void)state; }
static inline void widget_render_wpm_chart(const struct display_layout_block *b, const struct custom_status_state *state) { (void)b; (void)state; }
static inline void widget_wpm_tick(uint8_t current_wpm) { (void)current_wpm; }
static inline void widget_wpm_reset_history(void) {}
#endif

#if IS_ENABLED(CONFIG_SCYAN_WIDGET_BRANDING)
void widget_render_branding(const struct display_layout_block *b, const struct custom_status_state *state);
#else
static inline void widget_render_branding(const struct display_layout_block *b, const struct custom_status_state *state) { (void)b; (void)state; }
#endif

#if IS_ENABLED(CONFIG_SCYAN_WIDGET_SPLIT)
void widget_render_split(const struct display_layout_block *b, const struct custom_status_state *state);
#else
static inline void widget_render_split(const struct display_layout_block *b, const struct custom_status_state *state) { (void)b; (void)state; }
#endif

#if IS_ENABLED(CONFIG_SCYAN_WIDGET_SCREENSAVER)
void widget_render_screensaver(const struct display_layout_block *b, const struct custom_status_state *state);
#else
static inline void widget_render_screensaver(const struct display_layout_block *b, const struct custom_status_state *state) { (void)b; (void)state; }
#endif

#if IS_ENABLED(CONFIG_SCYAN_WIDGET_CAPS)
void widget_render_caps(const struct display_layout_block *b, const struct custom_status_state *state);
#else
static inline void widget_render_caps(const struct display_layout_block *b, const struct custom_status_state *state) { (void)b; (void)state; }
#endif

#if IS_ENABLED(CONFIG_SCYAN_WIDGET_BONGO)
void widget_render_bongo(const struct display_layout_block *b, const struct custom_status_state *state);
#else
static inline void widget_render_bongo(const struct display_layout_block *b, const struct custom_status_state *state) { (void)b; (void)state; }
#endif

#if IS_ENABLED(CONFIG_SCYAN_WIDGET_LOOP)
void widget_render_loop(const struct display_layout_block *b, const struct custom_status_state *state);
#else
static inline void widget_render_loop(const struct display_layout_block *b, const struct custom_status_state *state) { (void)b; (void)state; }
#endif

#if IS_ENABLED(CONFIG_SCYAN_WIDGET_TYPEWRITER)
void widget_render_typewriter(const struct display_layout_block *b, const struct custom_status_state *state);
void widget_typewriter_record_char(char c);
void widget_typewriter_record_key(uint16_t usage_page, uint32_t keycode, bool pressed);
void widget_typewriter_reset(void);
uint8_t widget_typewriter_get_len(void);
uint8_t widget_typewriter_get_random_count(void);
const char *widget_typewriter_get_buf(void);
#else
static inline void widget_render_typewriter(const struct display_layout_block *b, const struct custom_status_state *state) { (void)b; (void)state; }
static inline void widget_typewriter_record_char(char c) { (void)c; }
static inline void widget_typewriter_record_key(uint16_t usage_page, uint32_t keycode, bool pressed) { (void)usage_page; (void)keycode; (void)pressed; }
static inline void widget_typewriter_reset(void) {}
static inline uint8_t widget_typewriter_get_len(void) { return 0; }
static inline uint8_t widget_typewriter_get_random_count(void) { return 0; }
static inline const char *widget_typewriter_get_buf(void) { return ""; }
#endif

#if IS_ENABLED(CONFIG_SCYAN_WIDGET_KEYPRESS)
void widget_render_keypress(const struct display_layout_block *b, const struct custom_status_state *state);
void widget_keypress_record_key(uint16_t usage_page, uint32_t keycode, bool pressed);
void widget_keypress_reset(void);
#else
static inline void widget_render_keypress(const struct display_layout_block *b, const struct custom_status_state *state) { (void)b; (void)state; }
static inline void widget_keypress_record_key(uint16_t usage_page, uint32_t keycode, bool pressed) { (void)usage_page; (void)keycode; (void)pressed; }
static inline void widget_keypress_reset(void) {}
#endif

void widget_dispatch_block(const struct display_layout_block *b, const struct custom_status_state *state);
