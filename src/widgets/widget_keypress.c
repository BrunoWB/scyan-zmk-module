/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#include "widgets.h"
#include "canvas.h"
#include <string.h>

#define MAX_ACTIVE_KEYS 8

struct active_key_item {
    char name[16];
    uint32_t keycode;
    uint16_t usage_page;
};

static struct active_key_item active_keys[MAX_ACTIVE_KEYS];
static uint8_t active_key_count = 0;

static uint16_t last_rendered_symbol_id = 0;
static bool has_last_rendered_symbol = false;

void widget_keypress_reset(void) {
    active_key_count = 0;
    has_last_rendered_symbol = false;
    last_rendered_symbol_id = 0;
}

static void normalize_key_str(const char *src, char *dst, size_t dst_len) {
    if (!src || !dst || dst_len == 0) return;

    // Skip leading whitespace
    while (*src == ' ' || *src == '\t') src++;

    // Skip "&kp " if present
    if (strncmp(src, "&kp ", 4) == 0 || strncmp(src, "&KP ", 4) == 0) {
        src += 4;
        while (*src == ' ' || *src == '\t') src++;
    }

    size_t i = 0;
    while (*src && i < dst_len - 1) {
        char c = *src++;
        if (c >= 'A' && c <= 'Z') {
            c = c - 'A' + 'a';
        }
        dst[i++] = c;
    }
    // Trim trailing whitespace
    while (i > 0 && (dst[i - 1] == ' ' || dst[i - 1] == '\t')) {
        i--;
    }
    dst[i] = '\0';

    // Canonical aliases
    if (strcmp(dst, "spc") == 0 || strcmp(dst, " ") == 0) {
        strncpy(dst, "space", dst_len);
    } else if (strcmp(dst, "return") == 0 || strcmp(dst, "ret") == 0) {
        strncpy(dst, "enter", dst_len);
    } else if (strcmp(dst, "esc") == 0) {
        strncpy(dst, "escape", dst_len);
    } else if (strcmp(dst, "bspc") == 0) {
        strncpy(dst, "backspace", dst_len);
    } else if (strcmp(dst, "up") == 0 || strcmp(dst, "arrowup") == 0 || strcmp(dst, "▲") == 0) {
        strncpy(dst, "arrowup", dst_len);
    } else if (strcmp(dst, "down") == 0 || strcmp(dst, "arrowdown") == 0 || strcmp(dst, "▼") == 0) {
        strncpy(dst, "arrowdown", dst_len);
    } else if (strcmp(dst, "left") == 0 || strcmp(dst, "arrowleft") == 0 || strcmp(dst, "◀") == 0) {
        strncpy(dst, "arrowleft", dst_len);
    } else if (strcmp(dst, "right") == 0 || strcmp(dst, "arrowright") == 0 || strcmp(dst, "▶") == 0) {
        strncpy(dst, "arrowright", dst_len);
    } else if (strncmp(dst, "key", 3) == 0 && strlen(dst) == 4) {
        dst[0] = dst[3];
        dst[1] = '\0';
    } else if (strncmp(dst, "digit", 5) == 0 && strlen(dst) == 6) {
        dst[0] = dst[5];
        dst[1] = '\0';
    } else if (strncmp(dst, "numpad", 6) == 0 && strlen(dst) == 7) {
        dst[0] = dst[6];
        dst[1] = '\0';
    } else if (dst[0] == 'n' && dst[1] >= '0' && dst[1] <= '9' && dst[2] == '\0') {
        dst[0] = dst[1];
        dst[1] = '\0';
    }
}

static void hid_keycode_to_str(uint16_t usage_page, uint32_t keycode, char *dst, size_t dst_len) {
    if (!dst || dst_len == 0) return;
    dst[0] = '\0';
    if (usage_page != 0x07 && usage_page != 0) return;

    if (keycode >= 0x04 && keycode <= 0x1D) {
        dst[0] = 'a' + (char)(keycode - 0x04);
        dst[1] = '\0';
    } else if (keycode >= 0x1E && keycode <= 0x26) {
        dst[0] = '1' + (char)(keycode - 0x1E);
        dst[1] = '\0';
    } else if (keycode == 0x27) {
        dst[0] = '0';
        dst[1] = '\0';
    } else if (keycode == 0x28) {
        strncpy(dst, "enter", dst_len);
    } else if (keycode == 0x29) {
        strncpy(dst, "escape", dst_len);
    } else if (keycode == 0x2A) {
        strncpy(dst, "backspace", dst_len);
    } else if (keycode == 0x2B) {
        strncpy(dst, "tab", dst_len);
    } else if (keycode == 0x2C) {
        strncpy(dst, "space", dst_len);
    } else if (keycode == 0x4F) {
        strncpy(dst, "arrowright", dst_len);
    } else if (keycode == 0x50) {
        strncpy(dst, "arrowleft", dst_len);
    } else if (keycode == 0x51) {
        strncpy(dst, "arrowdown", dst_len);
    } else if (keycode == 0x52) {
        strncpy(dst, "arrowup", dst_len);
    }
}

void widget_keypress_record_key(uint16_t usage_page, uint32_t keycode, bool pressed) {
    char name[16];
    hid_keycode_to_str(usage_page, keycode, name, sizeof(name));
    if (name[0] == '\0') return;

    if (pressed) {
        for (uint8_t i = 0; i < active_key_count; i++) {
            if (active_keys[i].usage_page == usage_page && active_keys[i].keycode == keycode) {
                return;
            }
        }
        if (active_key_count < MAX_ACTIVE_KEYS) {
            strncpy(active_keys[active_key_count].name, name, sizeof(active_keys[active_key_count].name) - 1);
            active_keys[active_key_count].name[sizeof(active_keys[active_key_count].name) - 1] = '\0';
            active_keys[active_key_count].keycode = keycode;
            active_keys[active_key_count].usage_page = usage_page;
            active_key_count++;
        }
    } else {
        for (uint8_t i = 0; i < active_key_count; i++) {
            if ((active_keys[i].usage_page == usage_page && active_keys[i].keycode == keycode) ||
                strcmp(active_keys[i].name, name) == 0) {
                for (uint8_t j = i; j + 1 < active_key_count; j++) {
                    active_keys[j] = active_keys[j + 1];
                }
                active_key_count--;
                break;
            }
        }
    }
}

void widget_render_keypress(const struct display_layout_block *b, const struct custom_status_state *state) {
    if (!b || !b->enabled) return;
    (void)state;

    uint16_t symbol_to_render = 0;
    bool found_symbol = false;

    // 1. Check active keys (most recently pressed first)
    if (active_key_count > 0 && b->text_count > 0 && b->symbol_count > 0) {
        for (int k = (int)active_key_count - 1; k >= 0; k--) {
            const char *act_name = active_keys[k].name;
            for (uint8_t i = 0; i < b->text_count && i < b->symbol_count; i++) {
                if (!b->text_entries[i]) continue;
                char norm_entry[16];
                normalize_key_str(b->text_entries[i], norm_entry, sizeof(norm_entry));
                if (strcmp(norm_entry, act_name) == 0) {
                    symbol_to_render = b->symbol_ids[i];
                    found_symbol = true;
                    last_rendered_symbol_id = symbol_to_render;
                    has_last_rendered_symbol = true;
                    break;
                }
            }
            if (found_symbol) break;
        }
    }

    // 2. If no active key matched
    if (!found_symbol) {
        if (b->mode == 1) {
            // Idle symbol configured
            symbol_to_render = b->symbol_id;
            found_symbol = true;
        } else if (has_last_rendered_symbol) {
            // No idle symbol: persist last rendered key symbol
            symbol_to_render = last_rendered_symbol_id;
            found_symbol = true;
        } else if (b->symbol_count > 0) {
            // Initial fallback: first configured symbol
            symbol_to_render = b->symbol_ids[0];
            found_symbol = true;
        }
    }

    if (found_symbol && symbol_to_render < SYMBOL_COUNT) {
        const struct sprite_slice *s = &SYMBOL_SLICES[symbol_to_render];
        int dx = (b->width > s->width) ? b->x + (b->width - s->width) / 2 : b->x;
        int dy = (b->height > s->height) ? b->y + (b->height - s->height) / 2 : b->y;
        canvas_draw_slice(dx, dy, s);
    }
}
