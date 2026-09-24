/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include <zmk/endpoints.h>

#if __has_include("scyan_assets.h")
#include "scyan_assets.h"
#elif __has_include("scyan/scyan_assets.install.h")
#include "scyan/scyan_assets.install.h"
#elif __has_include("scyan_assets.install.h")
#include "scyan_assets.install.h"
#else
#error "scyan_assets.h not found! Please export your display layout from Scyan ZMK Studio into your zmk-config directory."
#endif

#ifndef CONFIG_SCYAN_IDLE_TIMEOUT_MS
#define CONFIG_SCYAN_IDLE_TIMEOUT_MS 10000
#endif

#ifndef SCYAN_IDLE_SCREENS_ENABLED
#define SCYAN_IDLE_SCREENS_ENABLED 1
#endif

#ifndef SCYAN_IDLE_SCREENS_ENABLED_LEFT
#define SCYAN_IDLE_SCREENS_ENABLED_LEFT SCYAN_IDLE_SCREENS_ENABLED
#endif

#ifndef SCYAN_IDLE_SCREENS_ENABLED_RIGHT
#define SCYAN_IDLE_SCREENS_ENABLED_RIGHT SCYAN_IDLE_SCREENS_ENABLED
#endif

#ifndef SCYAN_IDLE_TIMEOUT_MS
#define SCYAN_IDLE_TIMEOUT_MS CONFIG_SCYAN_IDLE_TIMEOUT_MS
#endif

#ifndef SCYAN_IDLE_TIMEOUT_MS_LEFT
#define SCYAN_IDLE_TIMEOUT_MS_LEFT SCYAN_IDLE_TIMEOUT_MS
#endif

#ifndef SCYAN_IDLE_TIMEOUT_MS_RIGHT
#define SCYAN_IDLE_TIMEOUT_MS_RIGHT SCYAN_IDLE_TIMEOUT_MS
#endif

#ifndef SCYAN_ROTATION
#if defined(DISPLAY_ROTATION)
#define SCYAN_ROTATION DISPLAY_ROTATION
#elif IS_ENABLED(CONFIG_SCYAN_ROTATION_270)
#define SCYAN_ROTATION 270
#elif IS_ENABLED(CONFIG_SCYAN_ROTATION_180)
#define SCYAN_ROTATION 180
#elif IS_ENABLED(CONFIG_SCYAN_ROTATION_0)
#define SCYAN_ROTATION 0
#else
#define SCYAN_ROTATION 90
#endif
#endif

#ifndef SCYAN_ROTATION_PERIPHERAL
#if defined(SCYAN_ROTATION_RIGHT)
#define SCYAN_ROTATION_PERIPHERAL SCYAN_ROTATION_RIGHT
#elif defined(DISPLAY_ROTATION_PERIPHERAL)
#define SCYAN_ROTATION_PERIPHERAL DISPLAY_ROTATION_PERIPHERAL
#elif defined(DISPLAY_ROTATION_RIGHT)
#define SCYAN_ROTATION_PERIPHERAL DISPLAY_ROTATION_RIGHT
#else
#define SCYAN_ROTATION_PERIPHERAL SCYAN_ROTATION
#endif
#endif

#ifndef SCYAN_ROTATION_LEFT
#define SCYAN_ROTATION_LEFT SCYAN_ROTATION
#endif
#ifndef SCYAN_ROTATION_RIGHT
#define SCYAN_ROTATION_RIGHT SCYAN_ROTATION_PERIPHERAL
#endif

#ifndef CONFIG_SCYAN_USER_NAME
#define CONFIG_SCYAN_USER_NAME "SCYAN"
#endif

#define CANVAS_WIDTH  DISPLAY_HW_WIDTH
#define CANVAS_HEIGHT DISPLAY_HW_HEIGHT

#ifndef MAX_BLOCK_SYMBOLS
#define MAX_BLOCK_SYMBOLS 16
#endif

#ifndef MAX_BLOCK_TEXTS
#define MAX_BLOCK_TEXTS 16
#endif

#ifndef SCYAN_WIDGET_TYPES_DEFINED
#define SCYAN_WIDGET_TYPES_DEFINED
enum display_widget_type {
    WIDGET_TYPE_NONE = 0,
    WIDGET_TYPE_OUTPUT_STATUS = 1,
    WIDGET_TYPE_BATTERY = 2,
    WIDGET_TYPE_LAYER = 3,
    WIDGET_TYPE_WPM = 4,
    WIDGET_TYPE_WPM_CHART = 5,
    WIDGET_TYPE_BRANDING = 6,
    WIDGET_TYPE_SPLIT = 7,
    WIDGET_TYPE_SCREENSAVER = 8,
    WIDGET_TYPE_CAPS_LOCK = 9,
    WIDGET_TYPE_BONGO = 10,
    WIDGET_TYPE_LOOP = 11,
    WIDGET_TYPE_TYPEWRITER = 12,
    WIDGET_TYPE_KEYPRESS = 13,
};

#define WIDGET_TYPE_ANIMATION WIDGET_TYPE_LOOP

struct display_layout_block {
    uint8_t type;
    int16_t x;
    int16_t y;
    uint8_t width;
    uint8_t height;
    bool enabled;
    uint8_t mode;
    int16_t param1;
    int16_t param2;
    int16_t param3;
    uint8_t symbol_count;
    uint16_t symbol_ids[MAX_BLOCK_SYMBOLS];
    uint8_t text_count;
    const char *text_entries[MAX_BLOCK_TEXTS];
    const char *custom_text;
    uint16_t symbol_id;
};
#endif

/**
 * Snapshot of keyboard runtime state for rendering.
 */
struct custom_status_state {
    uint8_t battery_level;
    bool charging;
    struct zmk_endpoint_instance selected_endpoint;
    int active_profile_index;
    bool active_profile_connected;
    bool active_profile_bonded;
    uint8_t active_layer;
    char layer_name[24];
    uint8_t wpm;
    bool split_connected;
    bool caps_lock;
    bool is_idle;
    uint8_t bongo_state; // 0: idle/neutral, 1: tap left, 2: tap right
    uint8_t wpm_tick;    // Increments on each time interval tick for scrolling WPM chart
    uint32_t loop_tick;  // Increments on animation frame interval for Loop widgets
    uint16_t keypress_count; // Increments on keystrokes for Typewriter/Keypress widgets
};
