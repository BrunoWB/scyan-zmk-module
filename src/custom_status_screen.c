/*
 * Custom status screen implementation for Corne vertical OLED display
 * matching custom pixel art design.
 */

#include <zephyr/kernel.h>
#include <string.h>
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

static void v_draw_bitmap(int x, int y, int w, int h, const uint8_t *bmp, int stride) {
    for (int r = 0; r < h; r++) {
        for (int c = 0; c < w; c++) {
            int byte_idx = r * stride + (c / 8);
            int bit_idx = 7 - (c % 8);
            if ((bmp[byte_idx] >> bit_idx) & 1) {
                v_set_pixel(x + c, y + r, 1);
            }
        }
    }
}

static void draw_battery_at(int x, int y, uint8_t level) {
    // Battery outer frame:
    // Top & bottom lines: x..x+15, y and y+9 (width 16)
    v_fill_rect(x, y, 16, 1, 1);
    v_fill_rect(x, y + 9, 16, 1, 1);
    // Left border: x, y+1..y+8
    v_fill_rect(x, y + 1, 1, 8, 1);
    // Right border: x+15, ONLY at y+1 and y+8 (y+2..y+7 is open to nipple)
    v_set_pixel(x + 15, y + 1, 1);
    v_set_pixel(x + 15, y + 8, 1);

    // Battery terminal/nipple: x+16, y+2..y+7 (height 6)
    v_fill_rect(x + 16, y + 2, 1, 6, 1);

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
        // Draw 3 digits at y=83: hundreds at x=2, tens at x=12, ones at x=22
        int d_hundreds = (wpm / 100) % 10;
        int d_tens = (wpm / 10) % 10;
        int d_ones = wpm % 10;
        v_draw_bitmap(2, 83, 8, 10, FONT_DIGITS[d_hundreds], 1);
        v_draw_bitmap(12, 83, 8, 10, FONT_DIGITS[d_tens], 1);
        v_draw_bitmap(22, 83, 8, 10, FONT_DIGITS[d_ones], 1);
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
        if (slot < num_arrows) {
            // Draw 5x3 arrowhead pointing right at y=95..99
            v_set_pixel(sx, 95, 1);
            v_set_pixel(sx, 96, 1);
            v_set_pixel(sx + 1, 96, 1);
            v_set_pixel(sx, 97, 1);
            v_set_pixel(sx + 1, 97, 1);
            v_set_pixel(sx + 2, 97, 1);
            v_set_pixel(sx, 98, 1);
            v_set_pixel(sx + 1, 98, 1);
            v_set_pixel(sx, 99, 1);
        } else {
            // Dot at center tip (sx + 2, y=97)
            v_set_pixel(sx + 2, 97, 1);
        }
    }
}

static void draw_idle_screen(const struct custom_status_state *state) {
    // 1. Skull looking straight forward (layer 0) at y=47..69, x=3..28 (26x23)
    v_draw_bitmap(3, 47, 26, 23, SKULL_LAYERS[0], 4);

    // 2. Custom pixel text at y=73..77
    const char *user_name = CONFIG_CUSTOM_STATUS_SCREEN_USER_NAME;
    int len = strlen(user_name);
    if (len > 0) {
        int total_w = 0;
        for (int i = 0; i < len; i++) {
            char c = user_name[i];
            if (c >= 'A' && c <= 'Z') {
                total_w += FONT_LETTERS[c - 'A'].width + 1;
            } else if (c >= 'a' && c <= 'z') {
                total_w += FONT_LETTERS[c - 'a'].width + 1;
            } else {
                total_w += 3;
            }
        }
        if (total_w > 0) total_w -= 1;
        int start_x = (DISPLAY_VIRTUAL_WIDTH - total_w + 1) / 2;
        if (start_x < 0) start_x = 0;
        int cur_x = start_x;
        for (int i = 0; i < len; i++) {
            char c = user_name[i];
            if (c >= 'A' && c <= 'Z') {
                int idx = c - 'A';
                v_draw_bitmap(cur_x, 73, FONT_LETTERS[idx].width, 5, FONT_LETTERS[idx].data, 1);
                cur_x += FONT_LETTERS[idx].width + 1;
            } else if (c >= 'a' && c <= 'z') {
                int idx = c - 'a';
                v_draw_bitmap(cur_x, 73, FONT_LETTERS[idx].width, 5, FONT_LETTERS[idx].data, 1);
                cur_x += FONT_LETTERS[idx].width + 1;
            } else {
                cur_x += 3;
            }
        }
    }

    // 3. Split connection icon at y=116..124, x=10..22 (13x9)
    if (state->split_connected) {
        v_draw_bitmap(10, 116, 13, 9, ICON_SPLIT_CONNECTED, 2);
    } else {
        v_draw_bitmap(10, 116, 13, 9, ICON_SPLIT_DISCONNECTED, 2);
    }
}

static void draw_qwerty_label(void) {
    // "QWERTY" pixel text at y=28..32 spanning x=0..31
    v_draw_bitmap(0, 28, FONT_LETTERS['Q' - 'A'].width, 5, FONT_LETTERS['Q' - 'A'].data, 1);
    v_draw_bitmap(5, 28, FONT_LETTERS['W' - 'A'].width, 5, FONT_LETTERS['W' - 'A'].data, 1);
    v_draw_bitmap(11, 28, FONT_LETTERS['E' - 'A'].width, 5, FONT_LETTERS['E' - 'A'].data, 1);
    v_draw_bitmap(16, 28, FONT_LETTERS['R' - 'A'].width, 5, FONT_LETTERS['R' - 'A'].data, 1);
    v_draw_bitmap(21, 28, FONT_LETTERS['T' - 'A'].width, 5, FONT_LETTERS['T' - 'A'].data, 1);
    v_draw_bitmap(27, 28, FONT_LETTERS['Y' - 'A'].width, 5, FONT_LETTERS['Y' - 'A'].data, 1);
}

static void draw_active_screen(const struct custom_status_state *state) {
    // 1. Connection icon (USB vs Bluetooth)
    if (state->selected_endpoint.transport == ZMK_TRANSPORT_USB) {
        // USB cable and connector icon at x=0, y=0 (12x10)
        v_draw_bitmap(0, 0, 12, 10, ICON_USB, 2);
    } else {
        // Bluetooth icon at x=2, y=3 (8x8)
        v_draw_bitmap(2, 3, 8, 8, ICON_BLUETOOTH, 1);

        // Profile Letter: only if connected
        if (state->active_profile_connected) {
            int prof = state->active_profile_index;
            if (prof >= 0 && prof < 26) {
                const struct glyph_letter *gl = &FONT_LETTERS[prof];
                v_draw_bitmap(8, 9, gl->width, 5, gl->data, 1);
            }
        }
    }

    // 2. Battery icon at x=13..29, y=3..12
    draw_battery(state->battery_level);

    // 3. Layer indicator: "QWERTY" text when on Qwerty base layer, else brackets
    uint8_t layer = state->active_layer;
    if (layer == 1) {
        draw_qwerty_label();
        v_draw_bitmap(3, 47, 26, 23, SKULL_LAYERS[0], 4);
    } else {
        uint8_t idx = 0;
        if (layer >= 2 && layer <= 4) {
            idx = layer - 1; // 2->1 (RightHold), 3->2 (LeftHold), 4->3 (SimmHold)
        }
        v_draw_bitmap(5, 25, 22, 11, BRACKET_LAYERS[idx], 3);
        v_draw_bitmap(3, 47, 26, 23, SKULL_LAYERS[idx], 4);
    }

    // 5. WPM digits and arrow progress bar at y=83..99
    draw_wpm_and_arrows(state->wpm);

    // 6. Split connection icon at y=116..124, x=10..22 (13x9)
    if (state->split_connected) {
        v_draw_bitmap(10, 116, 13, 9, ICON_SPLIT_CONNECTED, 2);
    } else {
        v_draw_bitmap(10, 116, 13, 9, ICON_SPLIT_DISCONNECTED, 2);
    }
}

#if IS_ENABLED(CONFIG_ZMK_SPLIT) && !IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
static void draw_peripheral_idle_screen(const struct custom_status_state *state) {
    // Idle screen: keep only the connection symbol at bottom (x=10, y=116, 13x9)
    if (state->split_connected) {
        v_draw_bitmap(10, 116, 13, 9, ICON_SPLIT_CONNECTED, 2);
    } else {
        v_draw_bitmap(10, 116, 13, 9, ICON_SPLIT_DISCONNECTED, 2);
    }
}

static void draw_peripheral_screen(const struct custom_status_state *state) {
    // 1. Centered battery icon at top (x=7, y=3) without USB or Bluetooth icon
    draw_battery_at(7, 3, state->battery_level);

    // 2. Module connection symbol at bottom (x=10, y=116, 13x9)
    if (state->split_connected) {
        v_draw_bitmap(10, 116, 13, 9, ICON_SPLIT_CONNECTED, 2);
    } else {
        v_draw_bitmap(10, 116, 13, 9, ICON_SPLIT_DISCONNECTED, 2);
    }
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

    // Map 32x128 virtual buffer to 128x32 hardware canvas buffer
    for (int vy = 0; vy < DISPLAY_VIRTUAL_HEIGHT; vy++) {
        for (int vx = 0; vx < DISPLAY_VIRTUAL_WIDTH; vx++) {
#if IS_ENABLED(CONFIG_CUSTOM_STATUS_SCREEN_ROTATION_270)
            int hx = vy;
            int hy = 31 - vx;
#else
            int hx = 127 - vy;
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
