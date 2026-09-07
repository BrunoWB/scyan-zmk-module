/*
 * Custom status screen implementation for Corne vertical OLED display
 * matching custom pixel art design.
 */

#include <zephyr/kernel.h>
#include <string.h>
#include <stdio.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <lvgl.h>
#include <zmk/display.h>
#include <zmk/display/status_screen.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/endpoint_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/events/position_state_changed.h>
#include <zmk/battery.h>
#include <zmk/endpoints.h>
#include <zmk/keymap.h>

#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
#include <zmk/usb.h>
#include <zmk/events/usb_conn_state_changed.h>
#endif

#if IS_ENABLED(CONFIG_ZMK_BLE)
#include <zmk/ble.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zephyr/bluetooth/conn.h>
#endif

#if IS_ENABLED(CONFIG_ZMK_WPM)
#include <zmk/wpm.h>
#include <zmk/events/wpm_state_changed.h>
#endif

#if IS_ENABLED(CONFIG_ZMK_SPLIT)
#include <zmk/events/split_peripheral_status_changed.h>
#include <zmk/split/transport/types.h>
#if IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
#include <zmk/split/transport/central.h>
#else
#include <zmk/split/bluetooth/peripheral.h>
#endif
#endif

#include "custom_display_assets.h"

#define CANVAS_WIDTH  DISPLAY_HW_WIDTH   // 128
#define CANVAS_HEIGHT DISPLAY_HW_HEIGHT  // 32

#ifndef CONFIG_CUSTOM_STATUS_SCREEN_IDLE_TIMEOUT_MS
#define CONFIG_CUSTOM_STATUS_SCREEN_IDLE_TIMEOUT_MS 10000
#endif

#ifndef CONFIG_CUSTOM_STATUS_SCREEN_USER_NAME
#define CONFIG_CUSTOM_STATUS_SCREEN_USER_NAME "SCYAN"
#endif

#if !defined(CONFIG_CUSTOM_STATUS_SCREEN_ROTATION_270) && !defined(CONFIG_CUSTOM_STATUS_SCREEN_ROTATION_90)
#define CONFIG_CUSTOM_STATUS_SCREEN_ROTATION_90 1
#endif

struct custom_status_state {
    uint8_t battery_level;
    bool charging;
    struct zmk_endpoint_instance selected_endpoint;
    int active_profile_index;
    bool active_profile_connected;
    bool active_profile_bonded;
    uint8_t active_layer;
    uint8_t wpm;
    bool split_connected;
    bool is_idle;
};

static lv_color_t canvas_buf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(CANVAS_WIDTH, CANVAS_HEIGHT)];
static lv_obj_t *canvas_obj;
static uint8_t vbuf[DISPLAY_VIRTUAL_HEIGHT][DISPLAY_VIRTUAL_WIDTH];
static int64_t last_activity_time = 0;
static bool is_screen_idle = false;
static bool initialized = false;

/* Virtual buffer drawing primitives */
static inline void v_clear(void) {
    memset(vbuf, 0, sizeof(vbuf));
}

static inline void v_set_pixel(int x, int y, uint8_t val) {
    if (x >= 0 && x < DISPLAY_VIRTUAL_WIDTH && y >= 0 && y < DISPLAY_VIRTUAL_HEIGHT) {
        vbuf[y][x] = val;
    }
}

static void v_fill_rect(int x, int y, int w, int h, uint8_t val) {
    for (int r = 0; r < h; r++) {
        for (int c = 0; c < w; c++) {
            v_set_pixel(x + c, y + r, val);
        }
    }
}

static void v_draw_atlas_subrect(int dst_x, int dst_y,
                                 const uint8_t *atlas, int atlas_stride,
                                 int src_x, int src_y, int w, int h) {
    for (int r = 0; r < h; r++) {
        int vy = dst_y + r;
        if (vy < 0 || vy >= DISPLAY_VIRTUAL_HEIGHT) continue;
        int sy = src_y + r;
        for (int c = 0; c < w; c++) {
            int vx = dst_x + c;
            if (vx < 0 || vx >= DISPLAY_VIRTUAL_WIDTH) continue;
            int sx = src_x + c;
            int byte_idx = sy * atlas_stride + (sx / 8);
            int bit_idx = 7 - (sx % 8);
            if ((atlas[byte_idx] >> bit_idx) & 1) {
                vbuf[vy][vx] = 1;
            }
        }
    }
}

static void v_draw_symbol(int dst_x, int dst_y, enum symbol_id id) {
    if (id >= SYMBOL_COUNT) return;
    const struct sprite_slice *slice = &SYMBOL_SLICES[id];
    v_draw_atlas_subrect(dst_x, dst_y,
                         SYMBOLS_ATLAS, SYMBOLS_ATLAS_STRIDE,
                         slice->x, slice->y, slice->width, slice->height);
}

static inline void v_draw_bitmap(int x, int y, int w, int h, const uint8_t *bmp, int stride) {
    v_draw_atlas_subrect(x, y, bmp, stride, 0, 0, w, h);
}

static uint16_t utf8_next_codepoint(const char **str) {
    if (!str || !*str || !**str) return 0;
    const uint8_t *s = (const uint8_t *)*str;
    uint16_t cp = 0;
    if (s[0] < 0x80) {
        cp = s[0];
        *str += 1;
    } else if ((s[0] & 0xE0) == 0xC0) {
        if ((s[1] & 0xC0) == 0x80) {
            cp = ((s[0] & 0x1F) << 6) | (s[1] & 0x3F);
            *str += 2;
        } else {
            *str += 1;
            return 0xFFFD;
        }
    } else if ((s[0] & 0xF0) == 0xE0) {
        if ((s[1] & 0xC0) == 0x80 && (s[2] & 0xC0) == 0x80) {
            cp = ((s[0] & 0x0F) << 12) | ((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
            *str += 3;
        } else {
            *str += 1;
            return 0xFFFD;
        }
    } else if ((s[0] & 0xF8) == 0xF0) {
        if ((s[1] & 0xC0) == 0x80 && (s[2] & 0xC0) == 0x80 && (s[3] & 0xC0) == 0x80) {
            *str += 4;
            return 0xFFFD;
        } else {
            *str += 1;
            return 0xFFFD;
        }
    } else {
        *str += 1;
        return 0xFFFD;
    }
    return cp;
}

static const struct font_glyph *v_font_find_glyph(const struct display_font *font, uint16_t codepoint) {
    if (!font || !font->glyphs) return NULL;

    for (uint16_t i = 0; i < font->glyph_count; i++) {
        if (font->glyphs[i].codepoint == codepoint) {
            return &font->glyphs[i];
        }
    }

    // Lowercase to uppercase fold (ASCII)
    if (codepoint >= 'a' && codepoint <= 'z') {
        uint16_t upper = codepoint - 'a' + 'A';
        for (uint16_t i = 0; i < font->glyph_count; i++) {
            if (font->glyphs[i].codepoint == upper) {
                return &font->glyphs[i];
            }
        }
    }

    // Lowercase to uppercase fold (Latin-1 Supplement: à..þ except ÷)
    if (codepoint >= 0x00E0 && codepoint <= 0x00FE && codepoint != 0x00F7) {
        uint16_t upper = codepoint - 0x0020;
        for (uint16_t i = 0; i < font->glyph_count; i++) {
            if (font->glyphs[i].codepoint == upper) {
                return &font->glyphs[i];
            }
        }
    }

    // Unicode accent fallback to base ASCII
    uint16_t base = 0;
    switch (codepoint) {
        case 0x00C0: case 0x00C1: case 0x00C2: case 0x00C3: case 0x00C4: case 0x00C5:
        case 0x00E0: case 0x00E1: case 0x00E2: case 0x00E3: case 0x00E4: case 0x00E5:
            base = 'A'; break;
        case 0x00C7: case 0x00E7:
            base = 'C'; break;
        case 0x00C8: case 0x00C9: case 0x00CA: case 0x00CB:
        case 0x00E8: case 0x00E9: case 0x00EA: case 0x00EB:
            base = 'E'; break;
        case 0x00CC: case 0x00CD: case 0x00CE: case 0x00CF:
        case 0x00EC: case 0x00ED: case 0x00EE: case 0x00EF:
            base = 'I'; break;
        case 0x00D1: case 0x00F1:
            base = 'N'; break;
        case 0x00D2: case 0x00D3: case 0x00D4: case 0x00D5: case 0x00D6:
        case 0x00F2: case 0x00F3: case 0x00F4: case 0x00F5: case 0x00F6:
            base = 'O'; break;
        case 0x00D9: case 0x00DA: case 0x00DB: case 0x00DC:
        case 0x00F9: case 0x00FA: case 0x00FB: case 0x00FC:
            base = 'U'; break;
        default: break;
    }
    if (base != 0) {
        for (uint16_t i = 0; i < font->glyph_count; i++) {
            if (font->glyphs[i].codepoint == base) {
                return &font->glyphs[i];
            }
        }
    }

    return NULL;
}

static int v_draw_char(int x, int y, const struct display_font *font, uint16_t codepoint) {
    const struct font_glyph *g = v_font_find_glyph(font, codepoint);
    if (!g) return 0;
    v_draw_atlas_subrect(x, y, font->atlas, font->atlas_stride,
                         g->x, g->y, g->width, g->height);
    return g->advance_x;
}

static int v_measure_text(const struct display_font *font, const char *text) {
    if (!font || !text) return 0;
    int total_w = 0;
    int last_trailing = 0;
    const char *ptr = text;
    while (*ptr) {
        uint16_t cp = utf8_next_codepoint(&ptr);
        if (cp == 0) break;
        if (cp == ' ') {
            total_w += font->space_advance ? font->space_advance : 3;
            last_trailing = 0;
            continue;
        }
        const struct font_glyph *g = v_font_find_glyph(font, cp);
        if (g) {
            total_w += g->advance_x;
            last_trailing = (g->advance_x > g->width) ? (g->advance_x - g->width) : 0;
        } else {
            total_w += 3;
            last_trailing = 0;
        }
    }
    if (total_w > 0 && last_trailing > 0) {
        total_w -= last_trailing;
    }
    return total_w;
}

static int v_draw_text(int x, int y, const struct display_font *font, const char *text) {
    if (!font || !text) return 0;
    int cur_x = x;
    const char *ptr = text;
    while (*ptr) {
        uint16_t cp = utf8_next_codepoint(&ptr);
        if (cp == 0) break;
        if (cp == ' ') {
            cur_x += font->space_advance ? font->space_advance : 3;
            continue;
        }
        const struct font_glyph *g = v_font_find_glyph(font, cp);
        if (g) {
            v_draw_atlas_subrect(cur_x, y, font->atlas, font->atlas_stride,
                                 g->x, g->y, g->width, g->height);
            cur_x += g->advance_x;
        } else {
            cur_x += 3;
        }
    }
    return cur_x - x;
}

static void draw_battery_at(int x, int y, uint8_t level) {
    // Battery outer frame from symbol atlas (17x10)
    v_draw_symbol(x, y, SYMBOL_BATTERY_FRAME);

    // Interior fill: rows y+2..y+7 (6 rows), columns start at x+2
    if (level >= 100) {
        // Special case 100%:
        // Top and bottom interior rows (y+2, y+7) have 12 pixels (x+2..x+13)
        // Middle rows (y+3..y+6) have 13 pixels (x+2..x+14) extending toward the nipple
        v_fill_rect(x + 2, y + 2, 12, 1, 1);
        v_fill_rect(x + 2, y + 3, 13, 4, 1);
        v_fill_rect(x + 2, y + 7, 12, 1, 1);
    } else if (level > 0) {
        // 0..99% is progressive across 72 total slots (12 cols x 6 rows, x+2..x+13)
        // Subtraction order: from top (row 0: y+2) down to bottom (row 5: y+7)
        int total_fill = (level * 72) / 99;
        if (total_fill > 72) total_fill = 72;
        if (total_fill < 1 && level > 0) total_fill = 1;

        int base_cols = total_fill / 6;
        int remainder = total_fill % 6;

        for (int r = 0; r < 6; r++) {
            int row_cols = base_cols + ((5 - r) < remainder ? 1 : 0);
            if (row_cols > 12) row_cols = 12;
            if (row_cols > 0) {
                v_fill_rect(x + 2, y + 2 + r, row_cols, 1, 1);
            }
        }
    }
}

static inline void draw_battery(uint8_t level) {
    draw_battery_at(13, 3, level);
}

static void draw_wpm_and_arrows(uint8_t wpm) {
    if (wpm > 0) {
        char buf[4];
        buf[0] = '0' + ((wpm / 100) % 10);
        buf[1] = '0' + ((wpm / 10) % 10);
        buf[2] = '0' + (wpm % 10);
        buf[3] = '\0';
        v_draw_text(2, 83, &font_digits, buf);
    }

    // Arrow indicator bar: 7 slots at y=95..99 (center row y=97)
    // 0 WPM: no arrows, only 7 dots
    // 100 WPM: all 7 arrows full
    int num_arrows = 0;
    if (wpm > 0) {
        num_arrows = (wpm >= 100) ? 7 : (wpm * 7 + 50) / 100;
        if (num_arrows < 1) num_arrows = 1;
        if (num_arrows > 7) num_arrows = 7;
    }

    for (int slot = 0; slot < 7; slot++) {
        int sx = 3 + slot * 4;
        v_draw_symbol(sx, 95, slot < num_arrows ? SYMBOL_ARROW_HEAD : SYMBOL_ARROW_DOT);
    }
}

static void draw_qwerty_label(void) {
    // "QWERTY" pixel text at y=28..32 spanning x=0..31
    v_draw_text(0, 28, &font_text, "QWERTY");
}

#if defined(HAS_CUSTOM_LAYOUT_BLOCKS)
#define WPM_HISTORY_MAX 32
static uint8_t wpm_history[WPM_HISTORY_MAX];
static uint8_t wpm_history_idx = 0;
static uint8_t wpm_history_count = 0;

static void update_wpm_history(uint8_t wpm) {
    wpm_history[wpm_history_idx] = wpm;
    wpm_history_idx = (wpm_history_idx + 1) % WPM_HISTORY_MAX;
    if (wpm_history_count < WPM_HISTORY_MAX) {
        wpm_history_count++;
    }
}

static void draw_wpm_chart(int x, int y, int w, int h, uint8_t current_wpm, int grid_size, int target_speed) {
    if (w <= 0 || h <= 0) return;
    if (target_speed <= 0) target_speed = 100;

    // Baseline dots
    for (int col = 0; col < w; col += 2) {
        v_set_pixel(x + col, y + h - 1, 1);
    }

    int num_samples = (wpm_history_count > 0) ? wpm_history_count : 1;
    int step = (wpm_history_count > 0 && wpm_history_count < w) ? (w / wpm_history_count) : 1;
    if (step < 1) step = 1;

    for (int i = 0; i < num_samples && i * step < w; i++) {
        int hist_pos = (wpm_history_count == WPM_HISTORY_MAX)
            ? (wpm_history_idx + i) % WPM_HISTORY_MAX
            : i;
        uint8_t sample_wpm = wpm_history[hist_pos];
        int bar_h = (sample_wpm * (h - 2)) / target_speed;
        if (bar_h > h - 2) bar_h = h - 2;
        if (bar_h < 1 && sample_wpm > 0) bar_h = 1;

        int px = x + i * step;
        for (int r = 0; r < bar_h; r++) {
            v_set_pixel(px, y + h - 2 - r, 1);
        }
    }
}

static void draw_block_widget(const struct display_layout_block *b, const struct custom_status_state *state) {
    if (!b || !b->enabled) return;

    switch (b->type) {
    case WIDGET_TYPE_OUTPUT_STATUS: {
        if (b->mode == 1) {
            if (state->selected_endpoint.transport == ZMK_TRANSPORT_USB) {
                v_draw_text(b->x, b->y, &font_text, b->custom_text ? b->custom_text : "USB");
            } else {
                char buf[8];
                snprintf(buf, sizeof(buf), "P%d", state->active_profile_index + 1);
                v_draw_text(b->x, b->y, &font_text, buf);
            }
        } else {
            if (state->selected_endpoint.transport == ZMK_TRANSPORT_USB) {
                v_draw_symbol(b->x, b->y, SYMBOL_USB);
            } else {
                v_draw_symbol(b->x, b->y, SYMBOL_BLUETOOTH);
                if (state->active_profile_connected) {
                    int prof = state->active_profile_index;
                    if (prof >= 0 && prof < 26) {
                        v_draw_char(b->x + 6, b->y + 6, &font_text, 'A' + prof);
                    }
                }
            }
        }
        break;
    }

    case WIDGET_TYPE_BATTERY: {
        if (b->mode == 1) {
            char buf[8];
            snprintf(buf, sizeof(buf), "%d%%", state->battery_level);
            v_draw_text(b->x, b->y, &font_text, buf);
        } else {
            draw_battery_at(b->x, b->y, state->battery_level);
        }
        break;
    }

    case WIDGET_TYPE_LAYER: {
        uint8_t layer = state->active_layer;
        if (b->mode == 1) {
            const char *names[] = { "QWERTY", "LOWER", "RAISE", "ADJUST" };
            const char *layer_name = (b->custom_text && b->custom_text[0]) ? b->custom_text : ((layer < 4) ? names[layer] : "OTHER");
            v_draw_text(b->x, b->y, &font_text, layer_name);
        } else {
            if (layer == 1) {
                v_draw_text(b->x, b->y, &font_text, "QWERTY");
            } else {
                uint8_t idx = 0;
                if (layer >= 2 && layer <= 4) {
                    idx = layer - 1;
                }
                v_draw_symbol(b->x, b->y, SYMBOL_BRACKET_LAYER(idx));
            }
        }
        break;
    }

    case WIDGET_TYPE_WPM: {
        if (b->mode == 1) {
            char buf[12];
            snprintf(buf, sizeof(buf), "%d WPM", state->wpm);
            v_draw_text(b->x, b->y, &font_text, buf);
        } else {
            if (state->wpm > 0) {
                char buf[4];
                buf[0] = '0' + ((state->wpm / 100) % 10);
                buf[1] = '0' + ((state->wpm / 10) % 10);
                buf[2] = '0' + (state->wpm % 10);
                buf[3] = '\0';
                v_draw_text(b->x, b->y, &font_digits, buf);
            }
            int num_arrows = 0;
            if (state->wpm > 0) {
                num_arrows = (state->wpm >= 100) ? 7 : (state->wpm * 7 + 50) / 100;
                if (num_arrows < 1) num_arrows = 1;
                if (num_arrows > 7) num_arrows = 7;
            }
            for (int slot = 0; slot < 7; slot++) {
                int sx = b->x + 1 + slot * 4;
                v_draw_symbol(sx, b->y + 12, slot < num_arrows ? SYMBOL_ARROW_HEAD : SYMBOL_ARROW_DOT);
            }
        }
        break;
    }

    case WIDGET_TYPE_WPM_CHART: {
        draw_wpm_chart(b->x, b->y, b->width, b->height, state->wpm, b->param1, b->param2);
        break;
    }

    case WIDGET_TYPE_BRANDING: {
        const char *text = (b->custom_text && b->custom_text[0]) ? b->custom_text : CONFIG_CUSTOM_STATUS_SCREEN_USER_NAME;
        int total_w = v_measure_text(&font_text, text);
        int sx = b->x;
        if (b->width > total_w) {
            sx = b->x + (b->width - total_w) / 2;
        }
        v_draw_text(sx, b->y, &font_text, text);
        break;
    }

    case WIDGET_TYPE_SPLIT: {
        v_draw_symbol(b->x, b->y, state->split_connected ? SYMBOL_SPLIT_CONNECTED : SYMBOL_SPLIT_DISCONNECTED);
        break;
    }

    case WIDGET_TYPE_SCREENSAVER: {
        enum symbol_id sym = (b->symbol_id < SYMBOL_COUNT) ? (enum symbol_id)b->symbol_id : SYMBOL_SKULL_LAYER_0;
        if (!is_idle && sym >= SYMBOL_SKULL_LAYER_0 && sym <= SYMBOL_SKULL_LAYER_3) {
            uint8_t layer = state->active_layer;
            if (layer == 1) {
                sym = SYMBOL_SKULL_LAYER_0;
            } else {
                uint8_t idx = 0;
                if (layer >= 2 && layer <= 4) {
                    idx = layer - 1;
                }
                sym = SYMBOL_SKULL_LAYER(idx);
            }
        }
        v_draw_symbol(b->x, b->y, sym);
        break;
    }

    case WIDGET_TYPE_CAPS_LOCK: {
        v_draw_text(b->x, b->y, &font_text, "CAPS");
        break;
    }

    default:
        break;
    }
}

static void draw_blocks(const struct display_layout_block *blocks, size_t count, const struct custom_status_state *state, bool is_idle) {
    for (size_t i = 0; i < count; i++) {
        if (!blocks[i].enabled) continue;
        draw_block_widget(&blocks[i], state, is_idle);
    }
}
#endif

static void draw_idle_screen(const struct custom_status_state *state) {
#if defined(HAS_CUSTOM_LAYOUT_BLOCKS)
    draw_blocks(LAYOUT_LEFT_IDLE_BLOCKS, LAYOUT_LEFT_IDLE_COUNT, state, true);
#else
    // 1. Skull looking straight forward (layer 0) at y=47..69, x=3..28 (26x23)
    v_draw_symbol(3, 47, SYMBOL_SKULL_LAYER_0);

    // 2. Custom pixel text at y=73..77
    const char *user_name = CONFIG_CUSTOM_STATUS_SCREEN_USER_NAME;
    int total_w = v_measure_text(&font_text, user_name);
    int start_x = (DISPLAY_VIRTUAL_WIDTH - total_w + 1) / 2;
    if (start_x < 0) start_x = 0;
    v_draw_text(start_x, 73, &font_text, user_name);

    // 3. Split connection icon at y=116..124, x=10..22 (13x9)
    v_draw_symbol(10, 116, state->split_connected ? SYMBOL_SPLIT_CONNECTED : SYMBOL_SPLIT_DISCONNECTED);
#endif
}

static void draw_active_screen(const struct custom_status_state *state) {
#if defined(HAS_CUSTOM_LAYOUT_BLOCKS)
    draw_blocks(LAYOUT_LEFT_ACTIVE_BLOCKS, LAYOUT_LEFT_ACTIVE_COUNT, state, false);
#else
    // 1. Connection icon (USB vs Bluetooth)
    if (state->selected_endpoint.transport == ZMK_TRANSPORT_USB) {
        // USB cable and connector icon at x=0, y=0 (12x10)
        v_draw_symbol(0, 0, SYMBOL_USB);
    } else {
        // Bluetooth icon at x=2, y=3 (8x8)
        v_draw_symbol(2, 3, SYMBOL_BLUETOOTH);

        // Profile Letter: only if connected
        if (state->active_profile_connected) {
            int prof = state->active_profile_index;
            if (prof >= 0 && prof < 26) {
                v_draw_char(8, 9, &font_text, 'A' + prof);
            }
        }
    }

    // 2. Battery icon at x=13..29, y=3..12
    draw_battery(state->battery_level);

    // 3. Layer indicator: "QWERTY" text when on Qwerty base layer, else brackets
    uint8_t layer = state->active_layer;
    if (layer == 1) {
        draw_qwerty_label();
        v_draw_symbol(3, 47, SYMBOL_SKULL_LAYER_0);
    } else {
        uint8_t idx = 0;
        if (layer >= 2 && layer <= 4) {
            idx = layer - 1; // 2->1 (RightHold), 3->2 (LeftHold), 4->3 (SimmHold)
        }
        v_draw_symbol(5, 25, SYMBOL_BRACKET_LAYER(idx));
        v_draw_symbol(3, 47, SYMBOL_SKULL_LAYER(idx));
    }

    // 5. WPM digits and arrow progress bar at y=83..99
    draw_wpm_and_arrows(state->wpm);

    // 6. Split connection icon at y=116..124, x=10..22 (13x9)
    v_draw_symbol(10, 116, state->split_connected ? SYMBOL_SPLIT_CONNECTED : SYMBOL_SPLIT_DISCONNECTED);
#endif
}

#if IS_ENABLED(CONFIG_ZMK_SPLIT) && !IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
static void draw_peripheral_idle_screen(const struct custom_status_state *state) {
#if defined(HAS_CUSTOM_LAYOUT_BLOCKS)
    draw_blocks(LAYOUT_RIGHT_IDLE_BLOCKS, LAYOUT_RIGHT_IDLE_COUNT, state, true);
#else
    // Idle screen: keep only the connection symbol at bottom (x=10, y=116, 13x9)
    v_draw_symbol(10, 116, state->split_connected ? SYMBOL_SPLIT_CONNECTED : SYMBOL_SPLIT_DISCONNECTED);
#endif
}

static void draw_peripheral_screen(const struct custom_status_state *state) {
#if defined(HAS_CUSTOM_LAYOUT_BLOCKS)
    draw_blocks(LAYOUT_RIGHT_ACTIVE_BLOCKS, LAYOUT_RIGHT_ACTIVE_COUNT, state, false);
#else
    // 1. Centered battery icon at top (x=7, y=3) without USB or Bluetooth icon
    draw_battery_at(7, 3, state->battery_level);

    // 2. Module connection symbol at bottom (x=10, y=116, 13x9)
    v_draw_symbol(10, 116, state->split_connected ? SYMBOL_SPLIT_CONNECTED : SYMBOL_SPLIT_DISCONNECTED);
#endif
}
#endif

static void render_screen(const struct custom_status_state *state) {
    if (!canvas_obj) return;

    v_clear();

#if IS_ENABLED(CONFIG_ZMK_SPLIT) && !IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    if (state->is_idle) {
        draw_peripheral_idle_screen(state);
    } else {
        draw_peripheral_screen(state);
    }
#else
    if (state->is_idle) {
        draw_idle_screen(state);
    } else {
        draw_active_screen(state);
    }
#endif

    // Map virtual buffer to hardware canvas buffer
    for (int vy = 0; vy < DISPLAY_VIRTUAL_HEIGHT; vy++) {
        for (int vx = 0; vx < DISPLAY_VIRTUAL_WIDTH; vx++) {
#if IS_ENABLED(CONFIG_CUSTOM_STATUS_SCREEN_ROTATION_270)
            int hx = vy;
            int hy = (DISPLAY_VIRTUAL_WIDTH - 1) - vx;
#else
            int hx = (DISPLAY_VIRTUAL_HEIGHT - 1) - vy;
            int hy = vx;
#endif
#if IS_ENABLED(CONFIG_CUSTOM_STATUS_SCREEN_INVERT)
            lv_canvas_set_px_color(canvas_obj, hx, hy,
                                   vbuf[vy][vx] ? lv_color_black() : lv_color_white());
#else
            lv_canvas_set_px_color(canvas_obj, hx, hy,
                                   vbuf[vy][vx] ? lv_color_white() : lv_color_black());
#endif
        }
    }

    lv_obj_invalidate(canvas_obj);
}

static struct custom_status_state last_rendered_state;
static bool state_has_rendered = false;

static void custom_status_update_cb(struct custom_status_state state) {
    if (state_has_rendered && memcmp(&last_rendered_state, &state, sizeof(state)) == 0) {
        return;
    }
#if defined(HAS_CUSTOM_LAYOUT_BLOCKS)
    if (!state_has_rendered || state.wpm != last_rendered_state.wpm) {
        update_wpm_history(state.wpm);
    }
#endif
    last_rendered_state = state;
    state_has_rendered = true;
    render_screen(&state);
}

// Forward declaration of idle_work
static struct k_work_delayable idle_work;

static struct custom_status_state custom_status_get_state(const zmk_event_t *eh) {
    struct custom_status_state s;
    memset(&s, 0, sizeof(s));

    // Battery status
    s.battery_level = zmk_battery_state_of_charge();
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    s.charging = zmk_usb_is_powered();
#endif

#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    // Endpoint status
    s.selected_endpoint = zmk_endpoints_selected();
#if IS_ENABLED(CONFIG_ZMK_BLE)
    s.active_profile_index = zmk_ble_active_profile_index();
    s.active_profile_connected = zmk_ble_active_profile_is_connected();
    s.active_profile_bonded = !zmk_ble_active_profile_is_open();
#endif

    // Layer status
    s.active_layer = zmk_keymap_highest_layer_active();

    // WPM status
#if IS_ENABLED(CONFIG_ZMK_WPM)
    s.wpm = zmk_wpm_get_state();
#endif

    // Split peripheral status (for central)
#if IS_ENABLED(CONFIG_ZMK_SPLIT)
    s.split_connected = false;
    STRUCT_SECTION_FOREACH(zmk_split_transport_central, t) {
        if (t->api && t->api->get_status) {
            struct zmk_split_transport_status st = t->api->get_status();
            if (st.connections != ZMK_SPLIT_TRANSPORT_CONNECTIONS_STATUS_DISCONNECTED) {
                s.split_connected = true;
                break;
            }
        }
    }
#else
    s.split_connected = false;
#endif

#else // Peripheral half
#if IS_ENABLED(CONFIG_ZMK_SPLIT)
    s.split_connected = zmk_split_bt_peripheral_is_connected();
#else
    s.split_connected = false;
#endif
#endif

    // Activity check: layer change resets idle timeout
#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    if (eh != NULL && as_zmk_layer_state_changed(eh) != NULL) {
        last_activity_time = k_uptime_get();
        if (is_screen_idle) {
            is_screen_idle = false;
        }
        k_work_reschedule(&idle_work, K_MSEC(CONFIG_CUSTOM_STATUS_SCREEN_IDLE_TIMEOUT_MS));
    }
#endif

    s.is_idle = is_screen_idle;
    return s;
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_custom_status, struct custom_status_state,
                            custom_status_update_cb, custom_status_get_state)

ZMK_SUBSCRIPTION(widget_custom_status, zmk_battery_state_changed);
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_custom_status, zmk_usb_conn_state_changed);
#endif
#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
ZMK_SUBSCRIPTION(widget_custom_status, zmk_endpoint_changed);
#if IS_ENABLED(CONFIG_ZMK_BLE)
ZMK_SUBSCRIPTION(widget_custom_status, zmk_ble_active_profile_changed);
#endif
ZMK_SUBSCRIPTION(widget_custom_status, zmk_layer_state_changed);
#if IS_ENABLED(CONFIG_ZMK_WPM)
ZMK_SUBSCRIPTION(widget_custom_status, zmk_wpm_state_changed);
#endif
#if IS_ENABLED(CONFIG_ZMK_SPLIT)
ZMK_SUBSCRIPTION(widget_custom_status, zmk_split_peripheral_status_changed);
#endif
#endif

/* Dedicated lightweight listener for idle wakeup; avoids triggering full display redraws during typing */
static int custom_idle_listener_cb(const zmk_event_t *eh) {
    if (as_zmk_position_state_changed(eh) != NULL) {
        last_activity_time = k_uptime_get();
        if (is_screen_idle) {
            is_screen_idle = false;
            if (zmk_display_is_initialized()) {
                widget_custom_status_refresh_state(NULL);
                k_work_submit_to_queue(zmk_display_work_q(), &widget_custom_status_work);
            }
        }
        k_work_reschedule(&idle_work, K_MSEC(CONFIG_CUSTOM_STATUS_SCREEN_IDLE_TIMEOUT_MS));
    }
    return ZMK_EV_EVENT_BUBBLE;
}
ZMK_LISTENER(custom_idle_listener, custom_idle_listener_cb);
ZMK_SUBSCRIPTION(custom_idle_listener, zmk_position_state_changed);

static void refresh_work_cb(struct k_work *work) {
    if (zmk_display_is_initialized()) {
        widget_custom_status_refresh_state(NULL);
        k_work_submit_to_queue(zmk_display_work_q(), &widget_custom_status_work);
    }
}
static K_WORK_DEFINE(refresh_work, refresh_work_cb);

static void idle_work_cb(struct k_work *work) {
    if (zmk_display_is_initialized()) {
        int64_t now = k_uptime_get();
        if (now - last_activity_time >= CONFIG_CUSTOM_STATUS_SCREEN_IDLE_TIMEOUT_MS) {
            is_screen_idle = true;
            widget_custom_status_refresh_state(NULL);
            k_work_submit_to_queue(zmk_display_work_q(), &widget_custom_status_work);
        } else {
            int64_t remaining = CONFIG_CUSTOM_STATUS_SCREEN_IDLE_TIMEOUT_MS - (now - last_activity_time);
            if (remaining <= 0) {
                remaining = 1;
            }
            k_work_reschedule(&idle_work, K_MSEC(remaining));
        }
    }
}
static K_WORK_DELAYABLE_DEFINE(idle_work, idle_work_cb);

#if IS_ENABLED(CONFIG_ZMK_BLE)
static void split_conn_cb(struct bt_conn *conn, uint8_t err) {
    k_work_submit(&refresh_work);
}
static void split_disconn_cb(struct bt_conn *conn, uint8_t reason) {
    k_work_submit(&refresh_work);
}
BT_CONN_CB_DEFINE(custom_split_conn_cb) = {
    .connected = split_conn_cb,
    .disconnected = split_disconn_cb,
};
#endif

static int custom_status_init(lv_obj_t *parent) {
    if (initialized) return 0;
    initialized = true;

    canvas_obj = lv_canvas_create(parent);
    lv_canvas_set_buffer(canvas_obj, canvas_buf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);
    lv_obj_align(canvas_obj, LV_ALIGN_TOP_LEFT, 0, 0);

    last_activity_time = k_uptime_get();
    is_screen_idle = false;
    k_work_schedule(&idle_work, K_MSEC(CONFIG_CUSTOM_STATUS_SCREEN_IDLE_TIMEOUT_MS));

    widget_custom_status_init();
    return 0;
}

lv_obj_t *zmk_display_status_screen(void) {
    lv_obj_t *screen = lv_obj_create(NULL);

    custom_status_init(screen);

    return screen;
}
