/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#include "widgets.h"
#include "canvas.h"
#include "font_renderer.h"
#include "engine.h"
#include <zephyr/kernel.h>
#include <string.h>

#define TYPEWRITER_BUF_SIZE 64
#define RANDOM_BANK_MAX 32

struct random_letter_item {
    char c;
    uint8_t x;
    uint8_t y;
    uint8_t font_size; // 0: small (5px), 1: big (10px)
    bool initialized;
};

struct typewriter_state {
    char buf[TYPEWRITER_BUF_SIZE];
    uint8_t len;
    uint32_t last_activity_time;

    // Persistent random bank
    struct random_letter_item random_bank[RANDOM_BANK_MAX];
    uint8_t random_count;
    uint32_t random_prng_state;
};

static struct typewriter_state tw_state;

static uint32_t tw_prng_next(void) {
    if (tw_state.random_prng_state == 0) {
        tw_state.random_prng_state = k_uptime_get_32() ^ 0x9e3779b1u;
        if (tw_state.random_prng_state == 0) tw_state.random_prng_state = 0x12345678u;
    }
    // 32-bit xorshift
    uint32_t x = tw_state.random_prng_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    tw_state.random_prng_state = x;
    return x;
}

void widget_typewriter_reset(void) {
    memset(&tw_state, 0, sizeof(tw_state));
}

uint8_t widget_typewriter_get_len(void) {
    return tw_state.len;
}

uint8_t widget_typewriter_get_random_count(void) {
    return tw_state.random_count;
}

const char *widget_typewriter_get_buf(void) {
    return tw_state.buf;
}

void widget_typewriter_record_char(char c) {
    tw_state.last_activity_time = k_uptime_get_32();

    if (c == '\b') {
        if (tw_state.len > 0) {
            tw_state.len--;
            tw_state.buf[tw_state.len] = '\0';
        }
        if (tw_state.random_count > 0) {
            tw_state.random_count--;
        }
        return;
    }

    if (c >= ' ' && c <= '~') {
        // Linear buffer for Spot & Inline modes
        if (tw_state.len < TYPEWRITER_BUF_SIZE - 1) {
            tw_state.buf[tw_state.len++] = c;
            tw_state.buf[tw_state.len] = '\0';
        } else {
            // Buffer full: shift left by 1 (FIFO)
            memmove(tw_state.buf, tw_state.buf + 1, TYPEWRITER_BUF_SIZE - 2);
            tw_state.buf[TYPEWRITER_BUF_SIZE - 2] = c;
            tw_state.buf[TYPEWRITER_BUF_SIZE - 1] = '\0';
        }

        // Random mode bank: append new printable letter
        if (c > ' ') {
            if (tw_state.random_count >= RANDOM_BANK_MAX) {
                memmove(&tw_state.random_bank[0], &tw_state.random_bank[1],
                        (RANDOM_BANK_MAX - 1) * sizeof(struct random_letter_item));
                tw_state.random_count = RANDOM_BANK_MAX - 1;
            }
            struct random_letter_item *item = &tw_state.random_bank[tw_state.random_count];
            item->c = c;
            item->initialized = false;
            tw_state.random_count++;
        }
    }
}

void widget_typewriter_record_key(uint16_t usage_page, uint32_t keycode, bool pressed) {
    if (!pressed) return;
    if (usage_page != 0x07 && usage_page != 0) return; // 0x07: HID_USAGE_KEY

    char c = 0;
    if (keycode >= 0x04 && keycode <= 0x1D) {
        c = 'A' + (char)(keycode - 0x04);
    } else if (keycode >= 0x1E && keycode <= 0x26) {
        c = '1' + (char)(keycode - 0x1E);
    } else if (keycode == 0x27) {
        c = '0';
    } else if (keycode == 0x28 || keycode == 0x2C) {
        c = ' ';
    } else if (keycode == 0x2A) {
        c = '\b';
    } else if (keycode == 0x2D) {
        c = '-';
    } else if (keycode == 0x2E) {
        c = '=';
    } else if (keycode == 0x2F) {
        c = '[';
    } else if (keycode == 0x30) {
        c = ']';
    } else if (keycode == 0x31) {
        c = '\\';
    } else if (keycode == 0x33) {
        c = ';';
    } else if (keycode == 0x34) {
        c = '\'';
    } else if (keycode == 0x35) {
        c = '`';
    } else if (keycode == 0x36) {
        c = ',';
    } else if (keycode == 0x37) {
        c = '.';
    } else if (keycode == 0x38) {
        c = '/';
    }

    if (c != 0) {
        widget_typewriter_record_char(c);
    }
}

void widget_render_typewriter(const struct display_layout_block *b, const struct custom_status_state *state) {
    if (!b || !b->enabled) return;
    (void)state;

    uint32_t now = k_uptime_get_32();
    uint32_t cleaning_ms = (b->param2 > 0) ? ((b->param2 <= 10) ? ((uint32_t)b->param2 * 1000) : (uint32_t)b->param2) : (b->mode == 2 ? 200 : 0);

    // 1. Idle auto-cleaning
    if (cleaning_ms > 0) {
        if (b->mode == 2) {
            // Random mode: incremental eviction of oldest letter
            if (tw_state.random_count > 0) {
                uint32_t elapsed = now - tw_state.last_activity_time;
                if (elapsed >= cleaning_ms) {
                    uint32_t letters_to_evict = elapsed / cleaning_ms;
                    if (letters_to_evict > tw_state.random_count) {
                        letters_to_evict = tw_state.random_count;
                    }
                    if (letters_to_evict >= tw_state.random_count) {
                        tw_state.random_count = 0;
                    } else {
                        memmove(&tw_state.random_bank[0], &tw_state.random_bank[letters_to_evict],
                                (tw_state.random_count - letters_to_evict) * sizeof(struct random_letter_item));
                        tw_state.random_count -= letters_to_evict;
                        tw_state.last_activity_time += letters_to_evict * cleaning_ms;
                    }
                }
                if (tw_state.random_count > 0) {
                    uint32_t next_in = (tw_state.last_activity_time + cleaning_ms > now)
                                       ? (tw_state.last_activity_time + cleaning_ms - now)
                                       : 10;
                    engine_schedule_typewriter_cleaning(next_in);
                }
            }
        } else {
            // Spot mode or inline mode: wipe buffer when cleaning timeout expires
            if (tw_state.len > 0) {
                uint32_t elapsed = now - tw_state.last_activity_time;
                if (elapsed >= cleaning_ms) {
                    tw_state.len = 0;
                    tw_state.buf[0] = '\0';
                } else {
                    engine_schedule_typewriter_cleaning(cleaning_ms - elapsed);
                }
            }
        }
    }

    if (b->mode == 1) {
        // Mode 1: Spot mode (single latest letter centered)
        const struct display_font *font = (b->param3 == 1) ? font_get_big() : font_get_small();
        int char_h = (b->param3 == 1) ? 10 : 5;

        char display_char = 0;
        if (tw_state.len > 0) {
            display_char = tw_state.buf[tw_state.len - 1];
        } else if (b->custom_text && b->custom_text[0] != '\0') {
            display_char = b->custom_text[0];
        }

        if (display_char > ' ' && display_char <= '~') {
            char str[2] = { display_char, '\0' };
            int cw = font_measure_text(font, str);
            int dx = b->x + ((b->width > cw) ? (b->width - cw) / 2 : 0);
            int dy = b->y + ((b->height > char_h) ? (b->height - char_h) / 2 : 0);
            font_draw_text(dx, dy, font, str);
        }
    } else if (b->mode == 2) {
        // Mode 2: Random mode (letters placed at persistent random coordinates)
        uint8_t capacity = (b->param1 > 0 && b->param1 <= RANDOM_BANK_MAX) ? (uint8_t)b->param1 : 20;

        // Trim to configured capacity
        if (tw_state.random_count > capacity) {
            uint8_t excess = tw_state.random_count - capacity;
            memmove(&tw_state.random_bank[0], &tw_state.random_bank[excess],
                    (tw_state.random_count - excess) * sizeof(struct random_letter_item));
            tw_state.random_count = capacity;
        }

        if (tw_state.random_count == 0 && b->custom_text && b->custom_text[0] != '\0') {
            const struct display_font *f = (b->param3 == 1) ? font_get_big() : font_get_small();
            font_draw_text(b->x + 2, b->y + 2, f, b->custom_text);
        } else {
            // Assign fixed random positions to newly added letters
            for (uint8_t i = 0; i < tw_state.random_count; i++) {
                struct random_letter_item *item = &tw_state.random_bank[i];
                if (!item->initialized) {
                    if (b->param3 == 1) {
                        item->font_size = 1; // big
                    } else if (b->param3 == 0) {
                        item->font_size = 0; // small
                    } else {
                        // param3 == 2 ('both'): pseudo-random
                        item->font_size = (tw_prng_next() & 1) ? 1 : 0;
                    }

                    const struct display_font *f = (item->font_size == 1) ? font_get_big() : font_get_small();
                    int char_h = (item->font_size == 1) ? 10 : 5;
                    char str[2] = { item->c, '\0' };
                    int cw = font_measure_text(f, str);

                    int max_off_x = (b->width > cw) ? (b->width - cw) : 0;
                    int max_off_y = (b->height > char_h) ? (b->height - char_h) : 0;

                    item->x = (max_off_x > 0) ? (uint8_t)(tw_prng_next() % (uint32_t)(max_off_x + 1)) : 0;
                    item->y = (max_off_y > 0) ? (uint8_t)(tw_prng_next() % (uint32_t)(max_off_y + 1)) : 0;
                    item->initialized = true;
                }

                // Render letter at its fixed position
                const struct display_font *font = (item->font_size == 1) ? font_get_big() : font_get_small();
                char str[2] = { item->c, '\0' };
                font_draw_text(b->x + item->x, b->y + item->y, font, str);
            }
        }
    } else {
        // Mode 0: Inline mode (horizontal or vertical text stream)
        const struct display_font *font = (b->param3 == 1) ? font_get_big() : font_get_small();
        int char_h = (b->param3 == 1) ? 10 : 5;

        const char *text = (tw_state.len > 0) ? tw_state.buf :
                           (b->custom_text ? b->custom_text : NULL);

        if (text && text[0] != '\0') {
            uint8_t dir = (uint8_t)b->param1; // 0: we, 1: ew, 2: ns, 3: sn

            if (dir == 0) { // West -> East (horizontal LTR with right-edge scroll)
                int total_w = font_measure_text(font, text);
                if (total_w <= b->width) {
                    font_draw_text(b->x, b->y, font, text);
                } else {
                    int tlen = (int)strlen(text);
                    int start_idx = 0;
                    for (int i = 0; i < tlen; i++) {
                        if (font_measure_text(font, text + i) <= b->width) {
                            start_idx = i;
                            break;
                        }
                    }
                    int drawn_w = font_measure_text(font, text + start_idx);
                    int dx = b->x + (b->width - drawn_w);
                    font_draw_text(dx, b->y, font, text + start_idx);
                }
            } else if (dir == 1) { // East -> West (horizontal RTL)
                int tlen = (int)strlen(text);
                char rev[TYPEWRITER_BUF_SIZE];
                if (tlen >= TYPEWRITER_BUF_SIZE) tlen = TYPEWRITER_BUF_SIZE - 1;
                for (int i = 0; i < tlen; i++) {
                    rev[i] = text[tlen - 1 - i];
                }
                rev[tlen] = '\0';

                int total_w = font_measure_text(font, rev);
                if (total_w <= b->width) {
                    int dx = b->x + (b->width - total_w);
                    font_draw_text(dx, b->y, font, rev);
                } else {
                    font_draw_text(b->x, b->y, font, rev);
                }
            } else if (dir == 2) { // North -> South (vertical top-to-bottom)
                int step_y = char_h + 1;
                int max_chars = (b->height + 1) / step_y;
                if (max_chars < 1) max_chars = 1;
                int tlen = (int)strlen(text);
                int start_idx = (tlen > max_chars) ? (tlen - max_chars) : 0;
                for (int i = start_idx; i < tlen; i++) {
                    char ch_str[2] = { text[i], '\0' };
                    int cy = b->y + (i - start_idx) * step_y;
                    if (cy + char_h > b->y + b->height) break;
                    int cw = font_measure_text(font, ch_str);
                    int cx = b->x + ((b->width > cw) ? (b->width - cw) / 2 : 0);
                    font_draw_text(cx, cy, font, ch_str);
                }
            } else { // South -> North (vertical bottom-to-top)
                int step_y = char_h + 1;
                int max_chars = (b->height + 1) / step_y;
                if (max_chars < 1) max_chars = 1;
                int tlen = (int)strlen(text);
                int start_idx = (tlen > max_chars) ? (tlen - max_chars) : 0;
                int num_chars = tlen - start_idx;
                for (int i = 0; i < num_chars; i++) {
                    char ch_str[2] = { text[start_idx + i], '\0' };
                    int cy = b->y + b->height - (num_chars - i) * step_y;
                    if (cy < b->y) continue;
                    int cw = font_measure_text(font, ch_str);
                    int cx = b->x + ((b->width > cw) ? (b->width - cw) / 2 : 0);
                    font_draw_text(cx, cy, font, ch_str);
                }
            }
        }
    }
}
