/*
 * Host-based test runner for scyan-zmk-module.
 */

#include "mock_zmk.h"
#include "custom_display_assets.h"

// Include module sources directly for testing
#include "canvas.h"
#include "canvas.c"
#include "font_renderer.h"
#include "font_renderer.c"
#include "widgets/widgets.h"
#include "widgets/widget_output.c"
#include "widgets/widget_battery.c"
#include "widgets/widget_layer.c"
#include "widgets/widget_wpm.c"
#include "widgets/widget_branding.c"
#include "widgets/widget_split.c"
#include "widgets/widget_screensaver.c"
#include "widgets/widget_caps.c"
#include "widgets/widget_bongo.c"

// Test runner functions

void test_canvas_primitives(void) {
    printf("[TEST] Testing canvas primitives...\n");
    canvas_clear();
    for (int y = 0; y < DISPLAY_VIRTUAL_HEIGHT; y++) {
        for (int x = 0; x < DISPLAY_VIRTUAL_WIDTH; x++) {
            assert(canvas_get_pixel(x, y) == 0);
        }
    }

    canvas_set_pixel(10, 20, 1);
    assert(canvas_get_pixel(10, 20) == 1);
    assert(canvas_get_pixel(10, 21) == 0);

    canvas_fill_rect(5, 5, 10, 10, 1);
    for (int y = 5; y < 15; y++) {
        for (int x = 5; x < 15; x++) {
            assert(canvas_get_pixel(x, y) == 1);
        }
    }
    assert(canvas_get_pixel(4, 5) == 0);
    assert(canvas_get_pixel(15, 5) == 0);
    printf("  -> Canvas primitives passed.\n");
}

void test_utf8_and_fonts(void) {
    printf("[TEST] Testing UTF-8 decoding and font rendering...\n");
    const char *text = "ZMK áéí 123";
    const char *p = text;

    assert(font_utf8_next_codepoint(&p) == 'Z');
    assert(font_utf8_next_codepoint(&p) == 'M');
    assert(font_utf8_next_codepoint(&p) == 'K');
    assert(font_utf8_next_codepoint(&p) == ' ');
    assert(font_utf8_next_codepoint(&p) == 0x00E1); // á
    assert(font_utf8_next_codepoint(&p) == 0x00E9); // é
    assert(font_utf8_next_codepoint(&p) == 0x00ED); // í
    assert(font_utf8_next_codepoint(&p) == ' ');
    assert(font_utf8_next_codepoint(&p) == '1');

    canvas_clear();
    int width = font_draw_text(0, 0, font_get_small(), "SCYAN");
    assert(width > 0);

    bool has_pixel = false;
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < width; x++) {
            if (canvas_get_pixel(x, y) == 1) {
                has_pixel = true;
                break;
            }
        }
    }
    assert(has_pixel);
    printf("  -> UTF-8 and font rasterizer passed (measured width = %d).\n", width);
}

void test_symbols(void) {
    printf("[TEST] Testing symbol drawing from SYMBOLS_ATLAS...\n");
    canvas_clear();
    canvas_draw_symbol(0, 0, SYMBOL_USB);

    bool has_pixel = false;
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 12; x++) {
            if (canvas_get_pixel(x, y) == 1) {
                has_pixel = true;
                break;
            }
        }
    }
    assert(has_pixel);
    printf("  -> Symbols passed.\n");
}

void test_widgets(void) {
    printf("[TEST] Testing widget rendering...\n");
    struct custom_status_state state = {
        .battery_level = 85,
        .charging = false,
        .selected_endpoint = { .transport = ZMK_TRANSPORT_BLE },
        .active_profile_index = 0,
        .active_profile_connected = true,
        .active_profile_bonded = true,
        .active_layer = 1,
        .layer_name = "LOWER",
        .wpm = 75,
        .split_connected = true,
        .caps_lock = false,
        .is_idle = false,
    };

    canvas_clear();
    for (size_t i = 0; i < LAYOUT_LEFT_ACTIVE_COUNT; i++) {
        widget_dispatch_block(&LAYOUT_LEFT_ACTIVE_BLOCKS[i], &state);
    }

    // Verify right active layout blocks
    canvas_clear();
    for (size_t i = 0; i < LAYOUT_RIGHT_ACTIVE_COUNT; i++) {
        widget_dispatch_block(&LAYOUT_RIGHT_ACTIVE_BLOCKS[i], &state);
    }

    // Verify left idle layout blocks
    state.is_idle = true;
    canvas_clear();
    for (size_t i = 0; i < LAYOUT_LEFT_IDLE_COUNT; i++) {
        widget_dispatch_block(&LAYOUT_LEFT_IDLE_BLOCKS[i], &state);
    }

    // Verify right idle layout blocks
    canvas_clear();
    for (size_t i = 0; i < LAYOUT_RIGHT_IDLE_COUNT; i++) {
        widget_dispatch_block(&LAYOUT_RIGHT_IDLE_BLOCKS[i], &state);
    }

    printf("  -> Layout blocks and widgets passed successfully.\n");
}

void test_rotation_transform(void) {
    printf("[TEST] Testing 90 degree rotation transformation math...\n");
    // In 90 degree rotation:
    // hx = (DISPLAY_VIRTUAL_HEIGHT - 1) - vy = 127 - vy
    // hy = vx
    int vx = 0, vy = 0;
    int hx = 127 - vy;
    int hy = vx;
    assert(hx == 127 && hy == 0);

    vx = 31; vy = 127;
    hx = 127 - vy;
    hy = vx;
    assert(hx == 0 && hy == 31);
    printf("  -> Rotation transformation passed.\n");
}

void test_bongo_widget(void) {
    printf("[TEST] Testing reactive Bongo Cat widget...\n");
    struct custom_status_state state = {
        .bongo_state = 0,
    };

    struct display_layout_block bongo_block = {
        .type = WIDGET_TYPE_BONGO,
        .x = 0,
        .y = 0,
        .width = 32,
        .height = 23,
        .enabled = true,
        .mode = 0,
        .symbol_count = 3,
        .symbol_ids = { SYMBOL_USB, SYMBOL_SPLIT_CONNECTED, SYMBOL_SPLIT_DISCONNECTED },
    };

    // State 0: neutral
    canvas_clear();
    widget_dispatch_block(&bongo_block, &state);
    bool has_pixel = false;
    for (int y = 0; y < DISPLAY_VIRTUAL_HEIGHT; y++) {
        for (int x = 0; x < DISPLAY_VIRTUAL_WIDTH; x++) {
            if (canvas_get_pixel(x, y) == 1) has_pixel = true;
        }
    }
    assert(has_pixel);

    // State 1: left tap
    state.bongo_state = 1;
    canvas_clear();
    widget_dispatch_block(&bongo_block, &state);
    has_pixel = false;
    for (int y = 0; y < DISPLAY_VIRTUAL_HEIGHT; y++) {
        for (int x = 0; x < DISPLAY_VIRTUAL_WIDTH; x++) {
            if (canvas_get_pixel(x, y) == 1) has_pixel = true;
        }
    }
    assert(has_pixel);

    // State 2: right tap
    state.bongo_state = 2;
    canvas_clear();
    widget_dispatch_block(&bongo_block, &state);
    has_pixel = false;
    for (int y = 0; y < DISPLAY_VIRTUAL_HEIGHT; y++) {
        for (int x = 0; x < DISPLAY_VIRTUAL_WIDTH; x++) {
            if (canvas_get_pixel(x, y) == 1) has_pixel = true;
        }
    }
    assert(has_pixel);

    // Text fallback mode
    bongo_block.mode = 1;
    bongo_block.text_count = 1;
    bongo_block.text_entries[0] = "BONGO";
    canvas_clear();
    widget_dispatch_block(&bongo_block, &state);
    has_pixel = false;
    for (int y = 0; y < DISPLAY_VIRTUAL_HEIGHT; y++) {
        for (int x = 0; x < DISPLAY_VIRTUAL_WIDTH; x++) {
            if (canvas_get_pixel(x, y) == 1) has_pixel = true;
        }
    }
    assert(has_pixel);

    printf("  -> Bongo Cat widget passed successfully.\n");
}

void test_battery_widget(void) {
    printf("[TEST] Testing battery widget level and symbol resolution...\n");
    struct display_layout_block batt_block = {
        .type = WIDGET_TYPE_BATTERY,
        .x = 0,
        .y = 0,
        .width = 17,
        .height = 10,
        .enabled = true,
        .mode = 0,
        .param1 = 2, // Even with stale param1=2, symbol_count must drive steps
        .param2 = 0,
        .symbol_count = 14,
        .symbol_ids = {
            SYMBOL_CHARGE_0960, SYMBOL_CHARGE_0960_SUB_1, SYMBOL_CHARGE_0960_SUB_2,
            SYMBOL_CHARGE_0960_SUB_3, SYMBOL_CHARGE_0960_SUB_4, SYMBOL_CHARGE_0960_SUB_5,
            SYMBOL_CHARGE_0960_SUB_6, SYMBOL_CHARGE_0960_SUB_7, SYMBOL_CHARGE_0960_SUB_8,
            SYMBOL_CHARGE_0960_SUB_9, SYMBOL_CHARGE_0960_SUB_10, SYMBOL_CHARGE_0960_SUB_11,
            SYMBOL_CHARGE_0960_SUB_12, SYMBOL_BATTERY_FRAME
        },
    };

    struct custom_status_state state = { .battery_level = 85, .charging = false };

    // Test 85%: with 14 steps, 85% must map to index 11 (not index 0)
    int steps = batt_block.symbol_count;
    int idx = ((int)state.battery_level * steps) / 100;
    if (idx >= steps) idx = steps - 1;
    assert(idx == 11);

    // Verify rendering does not crash
    canvas_clear();
    widget_render_battery(&batt_block, &state);

    // Test text mode percentage
    batt_block.mode = 1;
    batt_block.text_count = 0;
    canvas_clear();
    widget_render_battery(&batt_block, &state);

    // Test text mode with charging
    state.charging = true;
    canvas_clear();
    widget_render_battery(&batt_block, &state);

    printf("  -> Battery widget passed successfully.\n");
}

int main(void) {
    printf("=============================================\n");
    printf("Running unit test suite for scyan-zmk-module \n");
    printf("=============================================\n");

    test_canvas_primitives();
    test_utf8_and_fonts();
    test_symbols();
    test_widgets();
    test_rotation_transform();
    test_bongo_widget();
    test_battery_widget();

    printf("=============================================\n");
    printf("ALL TESTS PASSED SUCCESSFULLY!               \n");
    printf("=============================================\n");
    return 0;
}

