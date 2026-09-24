
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

typedef struct { uint16_t full; } lv_color_t;
static inline lv_color_t lv_color_white(void) { lv_color_t c = { 0xFFFF }; return c; }
static inline lv_color_t lv_color_black(void) { lv_color_t c = { 0x0000 }; return c; }
typedef struct lv_obj { uint8_t dummy; } lv_obj_t;

#define LV_CANVAS_BUF_SIZE_TRUE_COLOR(w, h) ((w) * (h))
#define LV_IMG_CF_TRUE_COLOR 0
#define LV_ALIGN_TOP_LEFT 0

static inline void lv_canvas_set_px_color(lv_obj_t *obj, int x, int y, lv_color_t color) { (void)obj; (void)x; (void)y; (void)color; }
static inline void lv_obj_invalidate(lv_obj_t *obj) { (void)obj; }

enum zmk_transport {
    ZMK_TRANSPORT_USB = 0,
    ZMK_TRANSPORT_BLE = 1,
};
struct zmk_endpoint_instance {
    enum zmk_transport transport;
};

#define IS_ENABLED(cfg) (cfg)
#define CONFIG_SCYAN_ROTATION_90 1
#define CONFIG_SCYAN_ROTATION_270 0
#define CONFIG_SCYAN_ROTATION_0 0
#define CONFIG_SCYAN_INVERT 1
#define CONFIG_SCYAN_IDLE_TIMEOUT_MS 10000
#define CONFIG_SCYAN_USER_NAME "SCYAN"
#define CONFIG_ZMK_SPLIT 1
#define CONFIG_ZMK_SPLIT_ROLE_CENTRAL 1

#define CONFIG_SCYAN_WIDGET_OUTPUT 1
#define CONFIG_SCYAN_WIDGET_BATTERY 1
#define CONFIG_SCYAN_WIDGET_LAYER 1
#define CONFIG_SCYAN_WIDGET_WPM 1
#define CONFIG_SCYAN_WIDGET_WPM_CHART 1
#define CONFIG_SCYAN_WIDGET_BRANDING 1
#define CONFIG_SCYAN_WIDGET_SPLIT 1
#define CONFIG_SCYAN_WIDGET_SCREENSAVER 1
#define CONFIG_SCYAN_WIDGET_CAPS 1
#define CONFIG_SCYAN_WIDGET_BONGO 1
#define CONFIG_SCYAN_WIDGET_LOOP 1
#define CONFIG_SCYAN_WIDGET_TYPEWRITER 1
#define CONFIG_SCYAN_WIDGET_KEYPRESS 1

struct k_work { void (*handler)(struct k_work *); };
struct k_work_delayable { struct k_work work; uint32_t timeout_ms; };
typedef uint32_t k_timeout_t;
#define K_MSEC(ms) (ms)
static inline void k_work_init_delayable(struct k_work_delayable *dwork, void (*handler)(struct k_work *)) {
    dwork->work.handler = handler;
}
static inline int k_work_schedule(struct k_work_delayable *dwork, k_timeout_t delay) {
    dwork->timeout_ms = delay;
    return 0;
}
static inline int k_work_reschedule(struct k_work_delayable *dwork, k_timeout_t delay) {
    dwork->timeout_ms = delay;
    return 0;
}
static inline int k_work_cancel_delayable(struct k_work_delayable *dwork) {
    dwork->timeout_ms = 0;
    return 0;
}

static uint32_t mock_uptime_ms = 100000;
static inline uint32_t k_uptime_get_32(void) { return mock_uptime_ms; }
static inline void mock_set_uptime(uint32_t ms) { mock_uptime_ms = ms; }

#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
#ifndef BUILD_ASSERT
#define BUILD_ASSERT(cond, msg) _Static_assert(cond, msg)
#endif

#define Z_COND_CODE_1(_ign, _then, ...) _then
#define Z_COND_CODE_0(_ign, _then, ...) __VA_ARGS__
#define ___COND_CODE_1(_flag, _then, ...) Z_COND_CODE_##_flag(_flag, _then, __VA_ARGS__)
#define __COND_CODE_1(_flag, _then, ...) ___COND_CODE_1(_flag, _then, __VA_ARGS__)
#define COND_CODE_1(_flag, _then, ...) __COND_CODE_1(_flag, _then, __VA_ARGS__)

#define DT_HAS_CHOSEN(node) 1
#define DT_CHOSEN(node) mock_display_1_active
#define DT_PHANDLE(node, prop) mock_display_1_idle

// mock_display_1_active
#define mock_display_1_active_NUM_CHILDREN 2
#define mock_display_1_active_HAS_PROP_idle_layout 1
#define mock_display_1_active_PROP_idle_timeout_ms 30000
#define mock_display_1_active_PROP_rotation 90
#define mock_display_1_active_CHILDREN(fn) fn(mock_act_w0) fn(mock_act_w1)

// mock_display_1_idle
#define mock_display_1_idle_NUM_CHILDREN 1
#define mock_display_1_idle_HAS_PROP_idle_layout 0
#define mock_display_1_idle_PROP_idle_timeout_ms 30000
#define mock_display_1_idle_PROP_rotation 90
#define mock_display_1_idle_CHILDREN(fn) fn(mock_idl_w0)

// mock_act_w0 (Battery)
#define mock_act_w0_COMPAT_scyan_widget_output 0
#define mock_act_w0_COMPAT_scyan_widget_battery 1
#define mock_act_w0_COMPAT_scyan_widget_layer 0
#define mock_act_w0_COMPAT_scyan_widget_wpm 0
#define mock_act_w0_COMPAT_scyan_widget_wpm_chart 0
#define mock_act_w0_COMPAT_scyan_widget_branding 0
#define mock_act_w0_COMPAT_scyan_widget_split 0
#define mock_act_w0_COMPAT_scyan_widget_screensaver 0
#define mock_act_w0_COMPAT_scyan_widget_caps 0
#define mock_act_w0_COMPAT_scyan_widget_bongo 0
#define mock_act_w0_COMPAT_scyan_widget_loop 0
#define mock_act_w0_COMPAT_scyan_widget_typewriter 0
#define mock_act_w0_COMPAT_scyan_widget_keypress 0
#define mock_act_w0_PROP_x 0
#define mock_act_w0_PROP_y 0
#define mock_act_w0_PROP_width 17
#define mock_act_w0_PROP_height 10
#define mock_act_w0_PROP_enabled 1
#define mock_act_w0_PROP_mode 0
#define mock_act_w0_PROP_param1 0
#define mock_act_w0_PROP_param2 0
#define mock_act_w0_PROP_param3 0
#define mock_act_w0_PROP_custom_text NULL
#define mock_act_w0_PROP_symbol_id 0
#define mock_act_w0_HAS_PROP_symbols 0
#define mock_act_w0_HAS_PROP_text_entries 0

// mock_act_w1 (Screensaver)
#define mock_act_w1_COMPAT_scyan_widget_output 0
#define mock_act_w1_COMPAT_scyan_widget_battery 0
#define mock_act_w1_COMPAT_scyan_widget_layer 0
#define mock_act_w1_COMPAT_scyan_widget_wpm 0
#define mock_act_w1_COMPAT_scyan_widget_wpm_chart 0
#define mock_act_w1_COMPAT_scyan_widget_branding 0
#define mock_act_w1_COMPAT_scyan_widget_split 0
#define mock_act_w1_COMPAT_scyan_widget_screensaver 1
#define mock_act_w1_COMPAT_scyan_widget_caps 0
#define mock_act_w1_COMPAT_scyan_widget_bongo 0
#define mock_act_w1_COMPAT_scyan_widget_loop 0
#define mock_act_w1_COMPAT_scyan_widget_typewriter 0
#define mock_act_w1_COMPAT_scyan_widget_keypress 0
#define mock_act_w1_PROP_x 0
#define mock_act_w1_PROP_y 20
#define mock_act_w1_PROP_width 32
#define mock_act_w1_PROP_height 32
#define mock_act_w1_PROP_enabled 1
#define mock_act_w1_PROP_mode 0
#define mock_act_w1_PROP_param1 0
#define mock_act_w1_PROP_param2 0
#define mock_act_w1_PROP_param3 0
#define mock_act_w1_PROP_custom_text NULL
#define mock_act_w1_PROP_symbol_id 8
#define mock_act_w1_HAS_PROP_symbols 0
#define mock_act_w1_HAS_PROP_text_entries 0

// mock_idl_w0 (Idle Screensaver)
#define mock_idl_w0_COMPAT_scyan_widget_output 0
#define mock_idl_w0_COMPAT_scyan_widget_battery 0
#define mock_idl_w0_COMPAT_scyan_widget_layer 0
#define mock_idl_w0_COMPAT_scyan_widget_wpm 0
#define mock_idl_w0_COMPAT_scyan_widget_wpm_chart 0
#define mock_idl_w0_COMPAT_scyan_widget_branding 0
#define mock_idl_w0_COMPAT_scyan_widget_split 0
#define mock_idl_w0_COMPAT_scyan_widget_screensaver 1
#define mock_idl_w0_COMPAT_scyan_widget_caps 0
#define mock_idl_w0_COMPAT_scyan_widget_bongo 0
#define mock_idl_w0_COMPAT_scyan_widget_loop 0
#define mock_idl_w0_COMPAT_scyan_widget_typewriter 0
#define mock_idl_w0_COMPAT_scyan_widget_keypress 0
#define mock_idl_w0_PROP_x 0
#define mock_idl_w0_PROP_y 0
#define mock_idl_w0_PROP_width 32
#define mock_idl_w0_PROP_height 64
#define mock_idl_w0_PROP_enabled 1
#define mock_idl_w0_PROP_mode 0
#define mock_idl_w0_PROP_param1 0
#define mock_idl_w0_PROP_param2 0
#define mock_idl_w0_PROP_param3 0
#define mock_idl_w0_PROP_custom_text NULL
#define mock_idl_w0_PROP_symbol_id 8
#define mock_idl_w0_HAS_PROP_symbols 0
#define mock_idl_w0_HAS_PROP_text_entries 0

#define _DT_NODE_HAS_COMPAT(node, compat) node ## _COMPAT_ ## compat
#define DT_NODE_HAS_COMPAT(node, compat) _DT_NODE_HAS_COMPAT(node, compat)

#define _DT_PROP_OR(node, prop, def) node ## _PROP_ ## prop
#define DT_PROP_OR(node, prop, def) _DT_PROP_OR(node, prop, def)

#define _DT_NODE_HAS_PROP(node, prop) node ## _HAS_PROP_ ## prop
#define DT_NODE_HAS_PROP(node, prop) _DT_NODE_HAS_PROP(node, prop)

#define _DT_FOREACH_CHILD(node, fn) node ## _CHILDREN(fn)
#define DT_FOREACH_CHILD(node, fn) _DT_FOREACH_CHILD(node, fn)

#define DT_PROP_LEN(node, prop) 0
#define DT_PROP_BY_IDX(node, prop, idx) 0
#define DT_FOREACH_PROP_ELEM(node, prop, fn)
