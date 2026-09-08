/*
 * Mock headers for host-side unit testing of scyan-zmk-module.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

typedef struct {
    uint16_t full;
} lv_color_t;

static inline lv_color_t lv_color_white(void) { lv_color_t c = { 0xFFFF }; return c; }
static inline lv_color_t lv_color_black(void) { lv_color_t c = { 0x0000 }; return c; }

typedef struct lv_obj {
    uint8_t dummy;
} lv_obj_t;

#define LV_CANVAS_BUF_SIZE_TRUE_COLOR(w, h) ((w) * (h))
#define LV_IMG_CF_TRUE_COLOR 0
#define LV_ALIGN_TOP_LEFT 0

enum zmk_transport {
    ZMK_TRANSPORT_USB = 0,
    ZMK_TRANSPORT_BLE = 1,
};

struct zmk_endpoint_instance {
    enum zmk_transport transport;
};

#define IS_ENABLED(cfg) (cfg)
#define CONFIG_CUSTOM_STATUS_SCREEN_ROTATION_90 1
#define CONFIG_CUSTOM_STATUS_SCREEN_ROTATION_270 0
#define CONFIG_CUSTOM_STATUS_SCREEN_ROTATION_0 0
#define CONFIG_CUSTOM_STATUS_SCREEN_INVERT 1
#define CONFIG_CUSTOM_STATUS_SCREEN_IDLE_TIMEOUT_MS 10000
#define CONFIG_CUSTOM_STATUS_SCREEN_LEFT_IS_CENTRAL 1
#define CONFIG_CUSTOM_STATUS_SCREEN_USER_NAME "SCYAN"
#define CONFIG_ZMK_SPLIT 1
#define CONFIG_ZMK_SPLIT_ROLE_CENTRAL 1

static uint32_t mock_uptime_ms = 100000;
static inline uint32_t k_uptime_get_32(void) {
    return mock_uptime_ms;
}
static inline void mock_set_uptime(uint32_t ms) {
    mock_uptime_ms = ms;
}

