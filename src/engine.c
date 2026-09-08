/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#include <string.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
#include "engine.h"
#include "canvas.h"
#include "transform.h"
#include "widgets/widgets.h"

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
    return true;
}

static void engine_render(const struct custom_status_state *state) {
    if (!engine_canvas_obj) return;

    canvas_clear();

    bool is_central;
#if IS_ENABLED(CONFIG_CUSTOM_STATUS_SCREEN_LEFT_IS_CENTRAL)
    is_central = (!IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL));
#else
    is_central = (IS_ENABLED(CONFIG_ZMK_SPLIT) && !IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL));
#endif

    const struct display_layout_block *blocks;
    size_t count;

    if (is_central) {
        if (state->is_idle) {
            blocks = LAYOUT_LEFT_IDLE_BLOCKS;
            count = LAYOUT_LEFT_IDLE_COUNT;
        } else {
            blocks = LAYOUT_LEFT_ACTIVE_BLOCKS;
            count = LAYOUT_LEFT_ACTIVE_COUNT;
        }
    } else {
        if (state->is_idle) {
            blocks = LAYOUT_RIGHT_IDLE_BLOCKS;
            count = LAYOUT_RIGHT_IDLE_COUNT;
        } else {
            blocks = LAYOUT_RIGHT_ACTIVE_BLOCKS;
            count = LAYOUT_RIGHT_ACTIVE_COUNT;
        }
    }

    for (size_t i = 0; i < count; i++) {
        widget_dispatch_block(&blocks[i], state);
    }

    transform_flush_to_lvgl_canvas(engine_canvas_obj);
}

void engine_update_state(struct custom_status_state state) {
    state.is_idle = is_screen_idle;

    if (state_has_rendered && states_equal(&state, &last_rendered_state)) {
        return;
    }

    last_rendered_state = state;
    state_has_rendered = true;

    engine_render(&state);
}

static void idle_work_cb(struct k_work *work) {
    if (!is_screen_idle) {
        is_screen_idle = true;
        engine_trigger_refresh();
    }
}

void engine_notify_activity(void) {
    if (is_screen_idle) {
        is_screen_idle = false;
        engine_trigger_refresh();
    }
    k_work_reschedule(&idle_work, K_MSEC(CONFIG_CUSTOM_STATUS_SCREEN_IDLE_TIMEOUT_MS));
}

bool engine_is_idle(void) {
    return is_screen_idle;
}

void engine_set_idle(bool idle) {
    is_screen_idle = idle;
}

void engine_init(lv_obj_t *canvas_obj) {
    engine_canvas_obj = canvas_obj;
    state_has_rendered = false;
    is_screen_idle = false;

    k_work_init_delayable(&idle_work, idle_work_cb);
    k_work_schedule(&idle_work, K_MSEC(CONFIG_CUSTOM_STATUS_SCREEN_IDLE_TIMEOUT_MS));
}

