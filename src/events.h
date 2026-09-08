/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "scyan/types.h"

void events_init(void);
struct custom_status_state events_get_current_state(const zmk_event_t *eh);

