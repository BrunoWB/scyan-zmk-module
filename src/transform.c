/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#include "transform.h"
#include "canvas.h"

void transform_flush_to_lvgl_canvas(lv_obj_t *canvas_obj) {
    if (!canvas_obj) return;

    const uint8_t (*vbuf)[DISPLAY_VIRTUAL_WIDTH] = canvas_get_vbuf();

    for (int vy = 0; vy < DISPLAY_VIRTUAL_HEIGHT; vy++) {
        for (int vx = 0; vx < DISPLAY_VIRTUAL_WIDTH; vx++) {
            int hx, hy;

#if IS_ENABLED(CONFIG_CUSTOM_STATUS_SCREEN_ROTATION_270)
            hx = vy;
            hy = (DISPLAY_VIRTUAL_WIDTH - 1) - vx;
#elif IS_ENABLED(CONFIG_CUSTOM_STATUS_SCREEN_ROTATION_0)
            hx = vx;
            hy = vy;
#else // Default: 90 degrees
            hx = (DISPLAY_VIRTUAL_HEIGHT - 1) - vy;
            hy = vx;
#endif

            if (hx < 0 || hx >= DISPLAY_HW_WIDTH || hy < 0 || hy >= DISPLAY_HW_HEIGHT) {
                continue;
            }

            uint8_t pixel_on = vbuf[vy][vx];

#if IS_ENABLED(CONFIG_CUSTOM_STATUS_SCREEN_INVERT)
            lv_color_t color = pixel_on ? lv_color_black() : lv_color_white();
#else
            lv_color_t color = pixel_on ? lv_color_white() : lv_color_black();
#endif

            lv_canvas_set_px_color(canvas_obj, hx, hy, color);
        }
    }

    lv_obj_invalidate(canvas_obj);
}

