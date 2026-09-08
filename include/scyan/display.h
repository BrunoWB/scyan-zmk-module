/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <lvgl.h>

/**
 * @brief Initialize and return the custom status screen LVGL object.
 *
 * Hooked by ZMK when CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM=y.
 */
lv_obj_t *zmk_display_status_screen(void);

