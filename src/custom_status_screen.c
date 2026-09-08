/*
 * Custom status screen implementation for ZMK display.
 * Purely data-driven rendering of layout blocks exported by zmk-display-builder.
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
    char layer_name[16];
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
        int vy = y + r;
        if (vy < 0 || vy >= DISPLAY_VIRTUAL_HEIGHT) continue;
        for (int c = 0; c < w; c++) {
            int vx = x + c;
            if (vx >= 0 && vx < DISPLAY_VIRTUAL_WIDTH) {
                vbuf[vy][vx] = val;
            }
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

static void v_draw_symbol(int dst_x, int dst_y, uint16_t id) {
    if (id >= SYMBOL_COUNT) return;
    const struct sprite_slice *slice = &SYMBOL_SLICES[id];
    v_draw_atlas_subrect(dst_x, dst_y,
                         SYMBOLS_ATLAS, SYMBOLS_ATLAS_STRIDE,
                         slice->x, slice->y, slice->width, slice->height);
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
            cp = ((s[0] & 0x07) << 18) | ((s[1] & 0x3F) << 12) | ((s[2] & 0x3F) << 6) | (s[3] & 0x3F);
            *str += 4;
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

static inline const struct display_font *v_get_text_font(void) {
    if (font_small.glyph_count > 0) return &font_small;
    if (font_text.glyph_count > 0) return &font_text;
    return &font_default;
}

static inline const struct display_font *v_get_digits_font(void) {
    if (font_digits.glyph_count > 0) return &font_digits;
    if (font_big.glyph_count > 0) return &font_big;
    return &font_default;
}

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

static void draw_wpm_chart(int x, int y, int w, int h, int grid_size, int target_speed) {
    if (w <= 0 || h <= 0) return;
    if (target_speed <= 0) target_speed = 100;

    // Grid baseline
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

static void draw_battery_widget(const struct display_layout_block *b, uint8_t level, bool charging) {
    if (b->mode == 1) {
        // Font mode
        if (b->text_count >= 2) {
            int idx = ((int)level * (b->text_count - 1) + 50) / 100;
            if (idx >= b->text_count) idx = b->text_count - 1;
            v_draw_text(b->x, b->y, v_get_text_font(), b->text_entries[idx]);
        } else {
            char buf[8];
            snprintf(buf, sizeof(buf), "%d%%", level);
            v_draw_text(b->x, b->y, v_get_text_font(), buf);
        }
        return;
    }

    // Symbol mode
    if (b->symbol_count >= 2) {
        // Progressive discrete battery symbols (e.g. 0%, 25%, 50%, 75%, 100%)
        int idx = ((int)level * (b->symbol_count - 1) + 50) / 100;
        if (idx >= b->symbol_count) idx = b->symbol_count - 1;
        v_draw_symbol(b->x, b->y, b->symbol_ids[idx]);
    } else {
        // Single battery frame with proportional fill
        uint16_t frame_sym = (b->symbol_count > 0) ? b->symbol_ids[0] : b->symbol_id;
        v_draw_symbol(b->x, b->y, frame_sym);

        const struct sprite_slice *s = (frame_sym < SYMBOL_COUNT) ? &SYMBOL_SLICES[frame_sym] : NULL;
        int fw = s ? s->width : b->width;
        int fh = s ? s->height : b->height;

        if (fw >= 10 && fh >= 6 && level > 0) {
            int inner_x = b->x + 2;
            int inner_y = b->y + 2;
            int inner_w = fw - 5;
            int inner_h = fh - 4;
            if (inner_w > 0 && inner_h > 0) {
                int fill_w = ((int)level * inner_w + 50) / 100;
                if (fill_w < 1 && level > 0) fill_w = 1;
                if (fill_w > inner_w) fill_w = inner_w;
                v_fill_rect(inner_x, inner_y, fill_w, inner_h, 1);
            }
        }
    }
}

static void draw_block_widget(const struct display_layout_block *b, const struct custom_status_state *state, bool is_idle) {
    if (!b || !b->enabled) return;

    switch (b->type) {
    case WIDGET_TYPE_OUTPUT_STATUS: {
        if (b->mode == 1) {
            // Font mode
            if (state->selected_endpoint.transport == ZMK_TRANSPORT_USB) {
                const char *txt = (b->text_count > 0 && b->text_entries[0]) ? b->text_entries[0] : (b->custom_text ? b->custom_text : "USB");
                v_draw_text(b->x, b->y, v_get_text_font(), txt);
            } else {
                int p = state->active_profile_index;
                if (b->text_count > p + 1 && b->text_entries[p + 1]) {
                    v_draw_text(b->x, b->y, v_get_text_font(), b->text_entries[p + 1]);
                } else {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "P%d", p + 1);
                    v_draw_text(b->x, b->y, v_get_text_font(), buf);
                }
            }
        } else {
            // Symbol mode
            if (state->selected_endpoint.transport == ZMK_TRANSPORT_USB) {
                uint16_t sym = (b->symbol_count > 0) ? b->symbol_ids[0] : b->symbol_id;
                v_draw_symbol(b->x, b->y, sym);
            } else {
                int p = state->active_profile_index;
                uint16_t sym;
                if (b->symbol_count > p + 1) {
                    sym = b->symbol_ids[p + 1];
                } else if (b->symbol_count > 1) {
                    sym = b->symbol_ids[1];
                } else if (b->symbol_count > 0) {
                    sym = b->symbol_ids[0];
                } else {
                    sym = b->symbol_id;
                }
                v_draw_symbol(b->x, b->y, sym);

                // If only generic BT icon is provided, overlay active profile letter/digit
                if (b->symbol_count <= 2 && state->active_profile_connected) {
                    if (p >= 0 && p < 26) {
                        v_draw_char(b->x + 6, b->y + 6, v_get_text_font(), 'A' + p);
                    }
                }
            }
        }
        break;
    }

    case WIDGET_TYPE_BATTERY: {
        draw_battery_widget(b, state->battery_level, state->charging);
        break;
    }

    case WIDGET_TYPE_LAYER: {
        uint8_t layer = state->active_layer;
        if (b->mode == 1) {
            // Font mode
            if (b->text_count > 0 && layer < b->text_count && b->text_entries[layer]) {
                v_draw_text(b->x, b->y, v_get_text_font(), b->text_entries[layer]);
            } else if (state->layer_name[0]) {
                v_draw_text(b->x, b->y, v_get_text_font(), state->layer_name);
            } else if (b->custom_text && b->custom_text[0]) {
                v_draw_text(b->x, b->y, v_get_text_font(), b->custom_text);
            } else {
                char buf[8];
                snprintf(buf, sizeof(buf), "L%d", layer);
                v_draw_text(b->x, b->y, v_get_text_font(), buf);
            }
        } else {
            // Symbol mode
            if (b->symbol_count > 0) {
                v_draw_symbol(b->x, b->y, b->symbol_ids[layer % b->symbol_count]);
            } else {
                v_draw_symbol(b->x, b->y, b->symbol_id);
            }
        }
        break;
    }

    case WIDGET_TYPE_WPM: {
        if (b->mode == 1) {
            // Font mode
            if (b->text_count >= 2) {
                int target = b->param2 > 0 ? b->param2 : 100;
                int idx = ((int)state->wpm * (b->text_count - 1)) / target;
                if (idx >= b->text_count) idx = b->text_count - 1;
                v_draw_text(b->x, b->y, v_get_text_font(), b->text_entries[idx]);
            } else {
                char buf[12];
                snprintf(buf, sizeof(buf), "%d WPM", state->wpm);
                v_draw_text(b->x, b->y, v_get_text_font(), buf);
            }
        } else {
            // Symbol mode
            if (b->symbol_count >= 2) {
                int target = b->param2 > 0 ? b->param2 : 100;
                int idx = ((int)state->wpm * (b->symbol_count - 1)) / target;
                if (idx >= b->symbol_count) idx = b->symbol_count - 1;
                v_draw_symbol(b->x, b->y, b->symbol_ids[idx]);
            } else {
                // Digits readout
                char buf[4];
                snprintf(buf, sizeof(buf), "%d", state->wpm);
                v_draw_text(b->x, b->y, v_get_digits_font(), buf);
            }
        }
        break;
    }

    case WIDGET_TYPE_WPM_CHART: {
        draw_wpm_chart(b->x, b->y, b->width, b->height, b->param1, b->param2);
        break;
    }

    case WIDGET_TYPE_BRANDING: {
        const char *text = (b->custom_text && b->custom_text[0])
            ? b->custom_text
            : ((b->text_count > 0 && b->text_entries[0]) ? b->text_entries[0] : CONFIG_CUSTOM_STATUS_SCREEN_USER_NAME);
        int total_w = v_measure_text(v_get_text_font(), text);
        int sx = b->x;
        if (b->width > total_w) {
            sx = b->x + (b->width - total_w) / 2;
        }
        v_draw_text(sx, b->y, v_get_text_font(), text);
        break;
    }

    case WIDGET_TYPE_SPLIT: {
        if (b->mode == 1) {
            // Font mode
            const char *txt = state->split_connected
                ? ((b->text_count > 0 && b->text_entries[0]) ? b->text_entries[0] : "OK")
                : ((b->text_count > 1 && b->text_entries[1]) ? b->text_entries[1] : "NC");
            v_draw_text(b->x, b->y, v_get_text_font(), txt);
        } else {
            // Symbol mode
            uint16_t sym = state->split_connected
                ? ((b->symbol_count > 0) ? b->symbol_ids[0] : b->symbol_id)
                : ((b->symbol_count > 1) ? b->symbol_ids[1] : b->symbol_id);
            v_draw_symbol(b->x, b->y, sym);
        }
        break;
    }

    case WIDGET_TYPE_SCREENSAVER: {
        if (b->symbol_count > 1) {
            v_draw_symbol(b->x, b->y, b->symbol_ids[state->active_layer % b->symbol_count]);
        } else if (b->symbol_count > 0) {
            v_draw_symbol(b->x, b->y, b->symbol_ids[0]);
        } else {
            v_draw_symbol(b->x, b->y, b->symbol_id);
        }
        break;
    }

    case WIDGET_TYPE_CAPS_LOCK: {
        if (b->mode == 1) {
            v_draw_text(b->x, b->y, v_get_text_font(), "CAPS");
        } else {
            uint16_t sym = (b->symbol_count > 0) ? b->symbol_ids[0] : b->symbol_id;
            v_draw_symbol(b->x, b->y, sym);
        }
        break;
    }

    default:
        break;
    }
}

static void draw_blocks(const struct display_layout_block *blocks, size_t count, const struct custom_status_state *state, bool is_idle) {
    if (!blocks) return;
    for (size_t i = 0; i < count; i++) {
        draw_block_widget(&blocks[i], state, is_idle);
    }
}

static void render_screen(const struct custom_status_state *state) {
    if (!canvas_obj) return;

    v_clear();

#if IS_ENABLED(CONFIG_ZMK_SPLIT) && !IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    if (state->is_idle) {
        draw_blocks(LAYOUT_RIGHT_IDLE_BLOCKS, LAYOUT_RIGHT_IDLE_COUNT, state, true);
    } else {
        draw_blocks(LAYOUT_RIGHT_ACTIVE_BLOCKS, LAYOUT_RIGHT_ACTIVE_COUNT, state, false);
    }
#else
    if (state->is_idle) {
        draw_blocks(LAYOUT_LEFT_IDLE_BLOCKS, LAYOUT_LEFT_IDLE_COUNT, state, true);
    } else {
        draw_blocks(LAYOUT_LEFT_ACTIVE_BLOCKS, LAYOUT_LEFT_ACTIVE_COUNT, state, false);
    }
#endif

    // Map virtual buffer to hardware canvas buffer with rotation & color inversion
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
    if (!state_has_rendered || state.wpm != last_rendered_state.wpm) {
        update_wpm_history(state.wpm);
    }
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
    // Central half: endpoints, layers, BLE, WPM, and split central status
    s.selected_endpoint = zmk_endpoints_selected();
#if IS_ENABLED(CONFIG_ZMK_BLE)
    s.active_profile_index = zmk_ble_active_profile_index();
    s.active_profile_connected = zmk_ble_active_profile_is_connected();
    s.active_profile_bonded = !zmk_ble_active_profile_is_open();
#endif

    // Layer status
    s.active_layer = zmk_keymap_highest_layer_active();
    const char *lname = zmk_keymap_layer_name(zmk_keymap_layer_index_to_id(s.active_layer));
    if (lname) {
        strncpy(s.layer_name, lname, sizeof(s.layer_name) - 1);
    }

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
#if IS_ENABLED(CONFIG_ZMK_SPLIT)
ZMK_SUBSCRIPTION(widget_custom_status, zmk_split_peripheral_status_changed);
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
