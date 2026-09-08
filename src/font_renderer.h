/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "scyan/types.h"

uint32_t font_utf8_next_codepoint(const char **str);
const struct font_glyph *font_find_glyph(const struct display_font *font, uint32_t codepoint);
int font_draw_char(int x, int y, const struct display_font *font, uint32_t codepoint);
int font_draw_text(int x, int y, const struct display_font *font, const char *text);
int font_measure_text(const struct display_font *font, const char *text);

const struct display_font *font_get_default(void);
const struct display_font *font_get_digits(void);
const struct display_font *font_get_small(void);
const struct display_font *font_get_big(void);

