/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "scyan/types.h"

void events_init(void);
void events_record_keystroke(void);
uint8_t events_get_local_wpm(void);

