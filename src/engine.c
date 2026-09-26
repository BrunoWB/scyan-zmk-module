/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#if __has_include(<zephyr/logging/log.h>)
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);
#else
#define LOG_MODULE_DECLARE(...)
#endif

#if __has_include(<zephyr/devicetree.h>)
#include <zephyr/devicetree.h>
#endif

#include <zmk/display.h>
#include "engine.h"
#include "canvas.h"
#include "transform.h"
#include "widgets/widgets.h"

#ifndef DT_HAS_CHOSEN
#define DT_HAS_CHOSEN(node) 0
#endif

static lv_obj_t *engine_canvas_obj = NULL;
static struct custom_status_state last_rendered_state;
static bool state_has_rendered = false;
static bool is_screen_idle = false;
static struct k_work_delayable idle_work;

static bool states_equal(const struct custom_status_state *a, const struct custom_status_state *b) {
    if (a->battery_level != b->battery_level) return false;
    if (a->charging != b->charging) return false;
    if (a->selected_endpoint.transport != b->selected_endpoint.transport) return false;
    if (a->active_profile_index != b->active_profile_index) return false;
    if (a->active_profile_connected != b->active_profile_connected) return false;
    if (a->active_profile_bonded != b->active_profile_bonded) return false;
    if (a->active_layer != b->active_layer) return false;
    if (strncmp(a->layer_name, b->layer_name, sizeof(a->layer_name)) != 0) return false;
    if (a->wpm != b->wpm) return false;
    if (a->split_connected != b->split_connected) return false;
    if (a->caps_lock != b->caps_lock) return false;
    if (a->is_idle != b->is_idle) return false;
    if (a->bongo_state != b->bongo_state) return false;
    if (a->wpm_tick != b->wpm_tick) return false;
    if (a->loop_tick != b->loop_tick) return false;
    if (a->keypress_count != b->keypress_count) return false;
    return true;
}

BUILD_ASSERT(DT_HAS_CHOSEN(scyan_display_layout),
             "Scyan ZMK: /chosen is missing 'scyan,display-layout'! Ensure overlay uses 'scyan,display-layout = &layout;' without angle brackets.");

#if DT_HAS_CHOSEN(scyan_display_layout)
#define CHOSEN_ACTIVE_LAYOUT DT_CHOSEN(scyan_display_layout)

#define _SCYAN_SYM_ELEM(node_id, prop, idx) DT_PROP_BY_IDX(node_id, prop, idx),
#define _SCYAN_TXT_ELEM(node_id, prop, idx) DT_PROP_BY_IDX(node_id, prop, idx),

#define SCYAN_WIDGET_TYPE(node) \
    (DT_NODE_HAS_COMPAT(node, scyan_widget_output) ? WIDGET_TYPE_OUTPUT_STATUS : \
     DT_NODE_HAS_COMPAT(node, scyan_widget_battery) ? WIDGET_TYPE_BATTERY : \
     DT_NODE_HAS_COMPAT(node, scyan_widget_layer) ? WIDGET_TYPE_LAYER : \
     DT_NODE_HAS_COMPAT(node, scyan_widget_wpm) ? WIDGET_TYPE_WPM : \
     DT_NODE_HAS_COMPAT(node, scyan_widget_wpm_chart) ? WIDGET_TYPE_WPM_CHART : \
     DT_NODE_HAS_COMPAT(node, scyan_widget_branding) ? WIDGET_TYPE_BRANDING : \
     DT_NODE_HAS_COMPAT(node, scyan_widget_split) ? WIDGET_TYPE_SPLIT : \
     DT_NODE_HAS_COMPAT(node, scyan_widget_screensaver) ? WIDGET_TYPE_SCREENSAVER : \
     DT_NODE_HAS_COMPAT(node, scyan_widget_caps) ? WIDGET_TYPE_CAPS_LOCK : \
     DT_NODE_HAS_COMPAT(node, scyan_widget_bongo) ? WIDGET_TYPE_BONGO : \
     DT_NODE_HAS_COMPAT(node, scyan_widget_loop) ? WIDGET_TYPE_LOOP : \
     DT_NODE_HAS_COMPAT(node, scyan_widget_typewriter) ? WIDGET_TYPE_TYPEWRITER : \
     DT_NODE_HAS_COMPAT(node, scyan_widget_keypress) ? WIDGET_TYPE_KEYPRESS : \
     WIDGET_TYPE_NONE)

#define SCYAN_DT_BLOCK_ITEM(node) \
    { \
        .type = SCYAN_WIDGET_TYPE(node), \
        .x = DT_PROP_OR(node, x, 0), \
        .y = DT_PROP_OR(node, y, 0), \
        .width = DT_PROP_OR(node, width, 0), \
        .height = DT_PROP_OR(node, height, 0), \
        .enabled = DT_PROP_OR(node, enabled, 1), \
        .mode = DT_PROP_OR(node, mode, 0), \
        .param1 = DT_PROP_OR(node, param1, 0), \
        .param2 = DT_PROP_OR(node, param2, 0), \
        .param3 = DT_PROP_OR(node, param3, 0), \
        .symbol_count = COND_CODE_1(DT_NODE_HAS_PROP(node, symbols), (DT_PROP_LEN(node, symbols)), (0)), \
        .symbol_ids = { \
            COND_CODE_1(DT_NODE_HAS_PROP(node, symbols), \
                (DT_FOREACH_PROP_ELEM(node, symbols, _SCYAN_SYM_ELEM)), (0)) \
        }, \
        .text_count = COND_CODE_1(DT_NODE_HAS_PROP(node, text_entries), (DT_PROP_LEN(node, text_entries)), (0)), \
        .text_entries = { \
            COND_CODE_1(DT_NODE_HAS_PROP(node, text_entries), \
                (DT_FOREACH_PROP_ELEM(node, text_entries, _SCYAN_TXT_ELEM)), (NULL)) \
        }, \
        .custom_text = DT_PROP_OR(node, custom_text, NULL), \
        .symbol_id = DT_PROP_OR(node, symbol_id, 0), \
    },

static const struct display_layout_block chosen_active_blocks[] = {
    DT_FOREACH_CHILD(CHOSEN_ACTIVE_LAYOUT, SCYAN_DT_BLOCK_ITEM)
    { .type = WIDGET_TYPE_NONE, .enabled = 0 }
};
#define CHOSEN_ACTIVE_COUNT (ARRAY_SIZE(chosen_active_blocks) - 1)

#if DT_NODE_HAS_PROP(CHOSEN_ACTIVE_LAYOUT, idle_layout)
#define CHOSEN_IDLE_LAYOUT DT_PHANDLE(CHOSEN_ACTIVE_LAYOUT, idle_layout)
static const struct display_layout_block chosen_idle_blocks[] = {
    DT_FOREACH_CHILD(CHOSEN_IDLE_LAYOUT, SCYAN_DT_BLOCK_ITEM)
    { .type = WIDGET_TYPE_NONE, .enabled = 0 }
};
#define CHOSEN_IDLE_COUNT (ARRAY_SIZE(chosen_idle_blocks) - 1)
#endif
#endif

static bool engine_idle_screens_enabled_for_half(void) {
#if DT_HAS_CHOSEN(scyan_display_layout)
#if DT_NODE_HAS_PROP(CHOSEN_ACTIVE_LAYOUT, idle_layout)
    return true;
#else
    return false;
#endif
#else
    return false;
#endif
}

static uint32_t engine_get_idle_timeout_ms_for_half(void) {
#if DT_HAS_CHOSEN(scyan_display_layout)
    return DT_PROP_OR(CHOSEN_ACTIVE_LAYOUT, idle_timeout_ms, 10000);
#else
    return 10000;
#endif
}

static int engine_get_rotation_for_half(void) {
#if DT_HAS_CHOSEN(scyan_display_layout)
    return DT_PROP_OR(CHOSEN_ACTIVE_LAYOUT, rotation, 90);
#else
    return 90;
#endif
}

static void engine_get_layout_blocks(bool show_idle, const struct display_layout_block **blocks_out, size_t *count_out) {
#if DT_HAS_CHOSEN(scyan_display_layout)
#if DT_NODE_HAS_PROP(CHOSEN_ACTIVE_LAYOUT, idle_layout)
    if (show_idle) {
        *blocks_out = chosen_idle_blocks;
        *count_out  = CHOSEN_IDLE_COUNT;
        return;
    }
#endif
    (void)show_idle;
    *blocks_out = chosen_active_blocks;
    *count_out  = CHOSEN_ACTIVE_COUNT;
#else
    (void)show_idle;
    *blocks_out = NULL;
    *count_out  = 0;
#endif
}

static void engine_render(const struct custom_status_state *state) {
    if (!engine_canvas_obj) return;

    canvas_clear();

    bool idle_enabled = engine_idle_screens_enabled_for_half();
    bool show_idle = state->is_idle && idle_enabled;

    const struct display_layout_block *blocks = NULL;
    size_t count = 0;
    engine_get_layout_blocks(show_idle, &blocks, &count);

    for (size_t i = 0; i < count; i++) {
        widget_dispatch_block(&blocks[i], state);
    }

    transform_flush_to_lvgl_canvas(engine_canvas_obj, engine_get_rotation_for_half());
}

static struct k_work_delayable loop_ticker_work;
static uint16_t engine_get_active_loop_speed(bool is_idle);
static inline uint32_t engine_calc_aligned_loop_delay(uint16_t speed);

void engine_update_state(struct custom_status_state state) {
    state.is_idle = is_screen_idle;

    if (state_has_rendered && states_equal(&state, &last_rendered_state)) {
        return;
    }

    last_rendered_state = state;
    state_has_rendered = true;

    uint16_t loop_speed = engine_get_active_loop_speed(state.is_idle);
    if (loop_speed > 0) {
        k_work_reschedule(&loop_ticker_work, K_MSEC(engine_calc_aligned_loop_delay(loop_speed)));
    }

    engine_render(&state);
}

static uint16_t engine_get_active_loop_speed(bool is_idle) {
    bool idle_enabled = engine_idle_screens_enabled_for_half();
    bool effective_idle = is_idle && idle_enabled;

    const struct display_layout_block *blocks = NULL;
    size_t count = 0;
    engine_get_layout_blocks(effective_idle, &blocks, &count);

    uint16_t min_speed = 0;
    for (size_t i = 0; i < count; i++) {
        if (blocks[i].enabled && blocks[i].type == WIDGET_TYPE_LOOP) {
            uint16_t speed = (blocks[i].param1 > 0) ? (uint16_t)blocks[i].param1 : 250;
            if (min_speed == 0 || speed < min_speed) {
                min_speed = speed;
            }
        }
        if (blocks[i].enabled && blocks[i].type == WIDGET_TYPE_OUTPUT_STATUS && blocks[i].symbol_count >= 13) {
            if (last_rendered_state.selected_endpoint.transport == ZMK_TRANSPORT_BLE &&
                !last_rendered_state.active_profile_connected) {
                uint16_t speed = 500;
                if (min_speed == 0 || speed < min_speed) {
                    min_speed = speed;
                }
            }
        }
    }
    return min_speed;
}

static uint32_t loop_ticker_state = 0;

static inline uint32_t engine_calc_aligned_loop_delay(uint16_t speed) {
    if (speed == 0) return 0;
    uint32_t now = k_uptime_get_32();
    uint32_t delay = speed - (now % speed);
    return (delay == 0) ? speed : delay;
}

static void loop_ticker_work_cb(struct k_work *work) {
    uint16_t speed = engine_get_active_loop_speed(is_screen_idle);
    if (speed > 0) {
        loop_ticker_state++;
        engine_trigger_refresh();
        k_work_reschedule(&loop_ticker_work, K_MSEC(engine_calc_aligned_loop_delay(speed)));
    }
}

uint32_t engine_get_loop_tick(void) {
    return loop_ticker_state;
}

static struct k_work_delayable wpm_ticker_work;

bool engine_has_wpm_chart(bool is_idle) {
    bool idle_enabled = engine_idle_screens_enabled_for_half();
    bool effective_idle = is_idle && idle_enabled;

    const struct display_layout_block *blocks = NULL;
    size_t count = 0;
    engine_get_layout_blocks(effective_idle, &blocks, &count);

    for (size_t i = 0; i < count; i++) {
        if (blocks[i].enabled && blocks[i].type == WIDGET_TYPE_WPM_CHART) {
            return true;
        }
    }
    return false;
}

static void idle_work_cb(struct k_work *work) {
    if (!engine_idle_screens_enabled_for_half()) {
        return;
    }
    if (!is_screen_idle) {
        is_screen_idle = true;
        loop_ticker_state = 0;
        engine_trigger_refresh();
        uint16_t idle_speed = engine_get_active_loop_speed(true);
        if (idle_speed > 0) {
            k_work_reschedule(&loop_ticker_work, K_MSEC(engine_calc_aligned_loop_delay(idle_speed)));
        } else {
            k_work_cancel_delayable(&loop_ticker_work);
        }
        if (engine_has_wpm_chart(true)) {
            k_work_reschedule(&wpm_ticker_work, K_MSEC(1000));
        } else {
            k_work_cancel_delayable(&wpm_ticker_work);
        }
    }
}

static uint8_t current_bongo_state = 0;
static struct k_work_delayable bongo_idle_work;

static void bongo_idle_work_cb(struct k_work *work) {
    if (current_bongo_state != 0) {
        current_bongo_state = 0;
        engine_trigger_refresh();
    }
}

static uint8_t wpm_ticker_state = 0;

static void wpm_ticker_work_cb(struct k_work *work) {
    if (engine_has_wpm_chart(is_screen_idle)) {
        wpm_ticker_state++;
        widget_wpm_tick(last_rendered_state.wpm);
        engine_trigger_refresh();
        k_work_reschedule(&wpm_ticker_work, K_MSEC(1000));
    }
}

uint8_t engine_get_wpm_tick(void) {
    return wpm_ticker_state;
}

#ifndef CONFIG_SCYAN_BONGO_TAP_MS
#define CONFIG_SCYAN_BONGO_TAP_MS 60
#endif

void engine_bongo_tap(bool is_left) {
    current_bongo_state = is_left ? 1 : 2;
    engine_trigger_refresh();
    k_work_reschedule(&bongo_idle_work, K_MSEC(CONFIG_SCYAN_BONGO_TAP_MS));
}

uint8_t engine_get_bongo_state(void) {
    return current_bongo_state;
}

void engine_notify_activity(void) {
    if (is_screen_idle) {
        is_screen_idle = false;
        loop_ticker_state = 0;
        engine_trigger_refresh();
        if (engine_has_wpm_chart(false)) {
            k_work_reschedule(&wpm_ticker_work, K_MSEC(1000));
        } else {
            k_work_cancel_delayable(&wpm_ticker_work);
        }
        uint16_t active_speed = engine_get_active_loop_speed(false);
        if (active_speed > 0) {
            k_work_reschedule(&loop_ticker_work, K_MSEC(engine_calc_aligned_loop_delay(active_speed)));
        } else {
            k_work_cancel_delayable(&loop_ticker_work);
        }
    }
    if (engine_idle_screens_enabled_for_half()) {
        uint32_t timeout_ms = engine_get_idle_timeout_ms_for_half();
        if (timeout_ms > 0) {
            k_work_reschedule(&idle_work, K_MSEC(timeout_ms));
        }
    }
}

bool engine_is_idle(void) {
    return is_screen_idle;
}

void engine_set_idle(bool idle) {
    is_screen_idle = idle;
}

static uint16_t keypress_counter = 0;
static struct k_work_delayable typewriter_cleaning_work;

static void typewriter_cleaning_work_cb(struct k_work *work) {
    (void)work;
    keypress_counter++;
    engine_trigger_refresh();
}

uint16_t engine_get_keypress_count(void) {
    return keypress_counter;
}

void engine_increment_keypress_count(void) {
    keypress_counter++;
}

void engine_schedule_typewriter_cleaning(uint32_t delay_ms) {
    if (delay_ms > 0) {
        k_work_reschedule(&typewriter_cleaning_work, K_MSEC(delay_ms));
    }
}

void engine_init(lv_obj_t *canvas_obj) {
    engine_canvas_obj = canvas_obj;
    state_has_rendered = false;
    is_screen_idle = false;
    current_bongo_state = 0;
    wpm_ticker_state = 0;
    loop_ticker_state = 0;
    keypress_counter = 0;

    k_work_init_delayable(&typewriter_cleaning_work, typewriter_cleaning_work_cb);

    k_work_init_delayable(&idle_work, idle_work_cb);
    if (engine_idle_screens_enabled_for_half()) {
        uint32_t timeout_ms = engine_get_idle_timeout_ms_for_half();
        if (timeout_ms > 0) {
            k_work_schedule(&idle_work, K_MSEC(timeout_ms));
        }
    }

    k_work_init_delayable(&bongo_idle_work, bongo_idle_work_cb);
    k_work_init_delayable(&wpm_ticker_work, wpm_ticker_work_cb);
    if (engine_has_wpm_chart(false)) {
        k_work_schedule(&wpm_ticker_work, K_MSEC(1000));
    }

    k_work_init_delayable(&loop_ticker_work, loop_ticker_work_cb);
    uint16_t init_loop_speed = engine_get_active_loop_speed(false);
    if (init_loop_speed > 0) {
        k_work_schedule(&loop_ticker_work, K_MSEC(engine_calc_aligned_loop_delay(init_loop_speed)));
    }
}
