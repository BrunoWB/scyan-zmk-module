/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#include "font_renderer.h"
#include "canvas.h"

uint32_t font_utf8_next_codepoint(const char **str) {
    if (!str || !*str || !**str) return 0;
    const uint8_t *p = (const uint8_t *)*str;
    uint32_t cp = 0;
    int len = 0;

    if (*p < 0x80) {
        cp = *p;
        len = 1;
    } else if ((*p & 0xE0) == 0xC0) {
        cp = *p & 0x1F;
        len = 2;
    } else if ((*p & 0xF0) == 0xE0) {
        cp = *p & 0x0F;
        len = 3;
    } else if ((*p & 0xF8) == 0xF0) {
        cp = *p & 0x07;
        len = 4;
    } else {
        *str += 1;
        return 0;
    }

    for (int i = 1; i < len; i++) {
        if ((p[i] & 0xC0) != 0x80) {
            *str += i;
            return 0;
        }
        cp = (cp << 6) | (p[i] & 0x3F);
    }

    *str += len;
    return cp;
}

const struct font_glyph *font_find_glyph(const struct display_font *font, uint32_t codepoint) {
    if (!font || !font->glyphs) return NULL;

    for (uint16_t i = 0; i < font->glyph_count; i++) {
        if (font->glyphs[i].codepoint == codepoint) {
            return &font->glyphs[i];
        }
    }

    // Case-insensitive letter fallback
    if (codepoint >= 'a' && codepoint <= 'z') {
        uint32_t upper = codepoint - 'a' + 'A';
        for (uint16_t i = 0; i < font->glyph_count; i++) {
            if (font->glyphs[i].codepoint == upper) {
                return &font->glyphs[i];
            }
        }
    } else if (codepoint >= 'A' && codepoint <= 'Z') {
        uint32_t lower = codepoint - 'A' + 'a';
        for (uint16_t i = 0; i < font->glyph_count; i++) {
            if (font->glyphs[i].codepoint == lower) {
                return &font->glyphs[i];
            }
        }
    }

    // If not found in requested font, search fallback in font_default
    if (font != &font_default) {
        return font_find_glyph(&font_default, codepoint);
    }

    return NULL;
}

int font_draw_char(int x, int y, const struct display_font *font, uint32_t codepoint) {
    if (!font) font = &font_default;

    if (codepoint == ' ') {
        return font->space_advance > 0 ? font->space_advance : 3;
    }

    const struct font_glyph *g = font_find_glyph(font, codepoint);
    if (!g) return 0;

    canvas_draw_atlas_subrect(x, y, font->atlas, font->atlas_stride,
                             g->x, g->y, g->width, g->height);
    return g->advance_x;
}

int font_draw_text(int x, int y, const struct display_font *font, const char *text) {
    if (!text) return 0;
    if (!font) font = &font_default;

    const char *p = text;
    int cur_x = x;
    uint32_t cp;

    while ((cp = font_utf8_next_codepoint(&p)) != 0) {
        cur_x += font_draw_char(cur_x, y, font, cp);
    }

    return cur_x - x;
}

int font_measure_text(const struct display_font *font, const char *text) {
    if (!text) return 0;
    if (!font) font = &font_default;

    const char *p = text;
    int cur_x = 0;
    int max_x = 0;
    uint32_t cp;

    while ((cp = font_utf8_next_codepoint(&p)) != 0) {
        if (cp == ' ') {
            cur_x += font->space_advance > 0 ? font->space_advance : 3;
            continue;
        }
        const struct font_glyph *g = font_find_glyph(font, cp);
        if (g) {
            int char_extent = cur_x + g->width;
            if (char_extent > max_x) {
                max_x = char_extent;
            }
            cur_x += g->advance_x;
        }
    }

    return max_x > 0 ? max_x : cur_x;
}

const struct display_font *font_get_default(void) {
    return &font_default;
}

const struct display_font *font_get_digits(void) {
    return &font_digits;
}

const struct display_font *font_get_small(void) {
    return &font_small;
}

const struct display_font *font_get_big(void) {
    return &font_big;
}

