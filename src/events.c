/*
 * Copyright (c) 2026 Bruno Barcellos
 * SPDX-License-Identifier: MIT
 */

#include <zephyr/kernel.h>
#include <string.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/display.h>
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

#if IS_ENABLED(CONFIG_ZMK_HID_INDICATORS)
#include <zmk/hid_indicators.h>
#endif

#include "events.h"
#include "engine.h"

static struct custom_status_state events_get_current_state(const zmk_event_t *eh) {
    struct custom_status_state s;
    memset(&s, 0, sizeof(s));

    // Battery status
    s.battery_level = zmk_battery_state_of_charge();
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    s.charging = zmk_usb_is_powered();
#endif

#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    // Central half: endpoints, layers, BLE, WPM, and split status
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

#if IS_ENABLED(CONFIG_ZMK_HID_INDICATORS)
    s.caps_lock = (zmk_hid_indicators_get_current_profile() & BIT(1)) != 0;
#endif

#else
    // Peripheral half
#if IS_ENABLED(CONFIG_ZMK_SPLIT)
    s.split_connected = zmk_split_bt_peripheral_is_connected();
#else
    s.split_connected = false;
#endif
#endif

    // Activity check: layer change resets idle timeout
#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    if (eh != NULL && as_zmk_layer_state_changed(eh) != NULL) {
        engine_notify_activity();
    }
#endif

    s.is_idle = engine_is_idle();
    s.bongo_state = engine_get_bongo_state();
    return s;
}

static void update_cb(struct custom_status_state state) {
    engine_update_state(state);
}

ZMK_DISPLAY_WIDGET_LISTENER(scyan_status_listener, struct custom_status_state,
                            update_cb, events_get_current_state)

ZMK_SUBSCRIPTION(scyan_status_listener, zmk_battery_state_changed);

#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(scyan_status_listener, zmk_usb_conn_state_changed);
#endif

#if IS_ENABLED(CONFIG_ZMK_SPLIT)
ZMK_SUBSCRIPTION(scyan_status_listener, zmk_split_peripheral_status_changed);
#endif

#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
ZMK_SUBSCRIPTION(scyan_status_listener, zmk_endpoint_changed);

#if IS_ENABLED(CONFIG_ZMK_BLE)
ZMK_SUBSCRIPTION(scyan_status_listener, zmk_ble_active_profile_changed);
#endif

ZMK_SUBSCRIPTION(scyan_status_listener, zmk_layer_state_changed);

#if IS_ENABLED(CONFIG_ZMK_WPM)
ZMK_SUBSCRIPTION(scyan_status_listener, zmk_wpm_state_changed);
#endif
#endif

/* Dedicated lightweight listener for idle wakeup and responsive bongo taps */
static int custom_activity_listener_cb(const zmk_event_t *eh) {
    const struct zmk_position_state_changed *pos_ev = as_zmk_position_state_changed(eh);
    if (pos_ev != NULL) {
        engine_notify_activity();
        if (pos_ev->state) {
            // Determine whether the key is from Left or Right side
#if !IS_ENABLED(CONFIG_ZMK_SPLIT)
            bool is_left = (pos_ev->position < 21);
#elif IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
            bool is_local = (pos_ev->source == ZMK_POSITION_STATE_CHANGE_SOURCE_LOCAL);
#if IS_ENABLED(CONFIG_CUSTOM_STATUS_SCREEN_LEFT_IS_CENTRAL)
            bool is_left = is_local;
#else
            bool is_left = !is_local;
#endif
#else
            // Peripheral half only receives its own local keystrokes
#if IS_ENABLED(CONFIG_CUSTOM_STATUS_SCREEN_LEFT_IS_CENTRAL)
            bool is_left = false;
#else
            bool is_left = true;
#endif
#endif
            engine_bongo_tap(is_left);
        }
    }
    return ZMK_EV_EVENT_BUBBLE;
}
ZMK_LISTENER(custom_activity_listener, custom_activity_listener_cb);
ZMK_SUBSCRIPTION(custom_activity_listener, zmk_position_state_changed);

#if IS_ENABLED(CONFIG_ZMK_BLE) && (!IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL))
static void split_conn_cb(struct bt_conn *conn, uint8_t err) {
    engine_trigger_refresh();
}
static void split_disconn_cb(struct bt_conn *conn, uint8_t reason) {
    engine_trigger_refresh();
}
BT_CONN_CB_DEFINE(custom_split_conn_cb) = {
    .connected = split_conn_cb,
    .disconnected = split_disconn_cb,
};
#endif

void engine_trigger_refresh(void) {
    if (zmk_display_is_initialized()) {
        scyan_status_listener_refresh_state(NULL);
        k_work_submit_to_queue(zmk_display_work_q(), &scyan_status_listener_work);
    }
}

void events_init(void) {
    scyan_status_listener_init();
}

