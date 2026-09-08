/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <lvgl.h>
#include "scyan/types.h"

/**
 * Maps the 32x128 virtual canvas into the 128x32 hardware LVGL canvas object,
 * applying configured rotation and color inversion.
 */
void transform_flush_to_lvgl_canvas(lv_obj_t *canvas_obj);

