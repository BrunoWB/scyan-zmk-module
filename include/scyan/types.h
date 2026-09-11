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

#ifndef HAS_CUSTOM_LAYOUT_BLOCKS
#error "scyan_assets.h must define HAS_CUSTOM_LAYOUT_BLOCKS. Please re-export your layout from Scyan ZMK Studio."
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

#ifndef CONFIG_SCYAN_USER_NAME
#define CONFIG_SCYAN_USER_NAME "SCYAN"
#endif

#define CANVAS_WIDTH  DISPLAY_HW_WIDTH   // 128
#define CANVAS_HEIGHT DISPLAY_HW_HEIGHT  // 32

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
};

#ifndef WIDGET_TYPE_BONGO
#define WIDGET_TYPE_BONGO 10
#endif

#ifndef WIDGET_TYPE_LOOP
#define WIDGET_TYPE_LOOP 11
#endif

#ifndef WIDGET_TYPE_ANIMATION
#define WIDGET_TYPE_ANIMATION WIDGET_TYPE_LOOP
#endif

