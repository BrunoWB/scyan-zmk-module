/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <lvgl.h>
#include <zephyr/kernel.h>
#include "scyan/types.h"

void engine_init(lv_obj_t *canvas_obj);
void engine_update_state(struct custom_status_state state);
void engine_trigger_refresh(void);
void engine_notify_activity(void);
bool engine_is_idle(void);
void engine_set_idle(bool idle);

