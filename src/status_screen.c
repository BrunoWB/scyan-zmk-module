/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#include <lvgl.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include "scyan/display.h"
#include "scyan/types.h"
#include "engine.h"
#include "events.h"

static lv_color_t canvas_buf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(CANVAS_WIDTH, CANVAS_HEIGHT)];
static lv_obj_t *canvas_obj = NULL;
static bool initialized = false;

static int status_screen_init(lv_obj_t *screen) {
    if (initialized) return 0;
    initialized = true;

    canvas_obj = lv_canvas_create(screen);
    lv_canvas_set_buffer(canvas_obj, canvas_buf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);
    lv_obj_align(canvas_obj, LV_ALIGN_TOP_LEFT, 0, 0);

    engine_init(canvas_obj);
    events_init();

    return 0;
}

lv_obj_t *zmk_display_status_screen(void) {
    lv_obj_t *screen = lv_obj_create(NULL);
    status_screen_init(screen);
    return screen;
}

