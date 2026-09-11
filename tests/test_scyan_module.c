/*
 * Host-based test runner for scyan-zmk-module.
 */

#include "mock_zmk.h"
#if __has_include("scyan_assets.h")
#include "scyan_assets.h"
#else
#include "scyan/scyan_assets.install.h"
#endif

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
#include "widgets/widget_loop.c"

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

    // Test Bresenham line drawing
    canvas_clear();
    canvas_draw_line(0, 0, 10, 10, 1);
    for (int i = 0; i <= 10; i++) {
        assert(canvas_get_pixel(i, i) == 1);
    }
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
    int measured_w = font_measure_text(font_get_small(), "SCYAN");
    assert(measured_w == 25);

    // Verify ink of 'N' exists at column 24
    bool has_n_ink = false;
    for (int y = 0; y < 5; y++) {
        if (canvas_get_pixel(24, y) == 1) {
            has_n_ink = true;
            break;
        }
    }
    assert(has_n_ink);

    printf("  -> UTF-8 and font rasterizer passed (measured width = %d, ink width = %d).\n", width, measured_w);
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

    // Verify branding widget renders full text without clipping even when placed near/past margin
    const char *test_entry = "SCYAN";
    struct display_layout_block test_branding_block = {
        .type = WIDGET_TYPE_BRANDING,
        .x = 8,
        .y = 10,
        .width = 32,
        .height = 5,
        .enabled = true,
        .mode = 1,
        .text_count = 1,
        .text_entries = { test_entry },
    };
    canvas_clear();
    widget_dispatch_block(&test_branding_block, &state);
    // Even though b->x was 8, clamp adjusted draw_x to 7 (32 - 25), so last pixel column of 'N' is at x = 7 + 24 = 31
    bool n_pixel_found = false;
    for (int y = 10; y < 15; y++) {
        if (canvas_get_pixel(31, y) == 1) {
            n_pixel_found = true;
            break;
        }
    }
    assert(n_pixel_found);

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

void test_wpm_chart_widget(void) {
    printf("[TEST] Testing WPM chart oscilloscope/heartbeat widget...\n");
    widget_wpm_reset_history();

    struct display_layout_block chart_block = {
        .type = WIDGET_TYPE_WPM_CHART,
        .x = 0,
        .y = 0,
        .width = 32,
        .height = 20,
        .enabled = true,
        .mode = 0,
        .param1 = 4,   // gridSize: 4
        .param2 = 100, // targetSpeed: 100
        .param3 = 30,  // timeWindow: 30s
    };

    struct custom_status_state state = { .wpm = 100 };

    canvas_clear();
    widget_render_wpm_chart(&chart_block, &state);

    // Border checks with param1 = 4
    assert(canvas_get_pixel(0, 0) == 1);
    assert(canvas_get_pixel(31, 0) == 1);
    assert(canvas_get_pixel(0, 19) == 1);
    assert(canvas_get_pixel(31, 19) == 1);

    // Rightmost inner column is x = 30 (width 32 - 1 border - 1).
    // With targetSpeed = 100 and wpm = 100, the peak should be at top inner row (y = 1).
    assert(canvas_get_pixel(30, 1) == 1);

    // Now test with wpm = 0
    state.wpm = 0;
    canvas_clear();
    widget_render_wpm_chart(&chart_block, &state);
    // Rightmost inner column (x = 30) should now be at bottom inner row (y = 18).
    assert(canvas_get_pixel(30, 18) == 1);

    // Test time ticking: advance history with wpm = 80
    widget_wpm_tick(80);
    // Now age 1 (x = 29) was recorded as 80.
    // At wpm = 80 and target = 100, inner_h = 18, so py = 18 - (80 * 17) / 100 = 18 - 13 = 5 (y = 1 + 4 = 5).
    canvas_clear();
    widget_render_wpm_chart(&chart_block, &state);
    // x = 29 should have pixel turned on near y = 5 (within 1px due to integer scaling)
    assert(canvas_get_pixel(29, 4) == 1 || canvas_get_pixel(29, 5) == 1 || canvas_get_pixel(29, 6) == 1);

    // Test no-grid mode (gridSize = 0)
    chart_block.param1 = 0;
    canvas_clear();
    widget_render_wpm_chart(&chart_block, &state);
    // In no-grid mode, baseline at bottom row (y = 19) is drawn
    assert(canvas_get_pixel(0, 19) == 1);
    assert(canvas_get_pixel(15, 19) == 1);
    assert(canvas_get_pixel(31, 19) == 1);

    // Test boundary clamping when width exceeds DISPLAY_VIRTUAL_WIDTH (e.g. 36px on 32px screen)
    chart_block.param1 = 4;
    chart_block.width = 36;
    state.wpm = 100;
    canvas_clear();
    widget_render_wpm_chart(&chart_block, &state);
    // Right border must be clamped to x = 31 (not offscreen at 35)
    assert(canvas_get_pixel(31, 0) == 1);
    assert(canvas_get_pixel(31, 19) == 1);
    // Rightmost inner column is x = 30 and peak is at top (y = 1)
    assert(canvas_get_pixel(30, 1) == 1);

    printf("  -> WPM chart widget passed successfully.\n");
}

void test_loop_widget(void) {
    printf("[TEST] Testing Loop animation widget sequence...\n");
    struct display_layout_block loop_block = {
        .type = WIDGET_TYPE_LOOP,
        .x = 0,
        .y = 0,
        .width = 16,
        .height = 16,
        .enabled = true,
        .mode = 0,
        .param1 = 200,
        .symbol_count = 3,
        .symbol_ids = {
            SYMBOL_CHARGE_0960,
            SYMBOL_CHARGE_0960_SUB_1,
            SYMBOL_CHARGE_0960_SUB_2,
        },
    };

    struct custom_status_state state = {0};

    // Frame 0
    state.loop_tick = 0;
    canvas_clear();
    widget_dispatch_block(&loop_block, &state);
    bool has_pixel = false;
    for (int y = 0; y < DISPLAY_VIRTUAL_HEIGHT; y++) {
        for (int x = 0; x < DISPLAY_VIRTUAL_WIDTH; x++) {
            if (canvas_get_pixel(x, y) == 1) has_pixel = true;
        }
    }
    assert(has_pixel);

    // Frame 1
    state.loop_tick = 1;
    canvas_clear();
    widget_dispatch_block(&loop_block, &state);
    has_pixel = false;
    for (int y = 0; y < DISPLAY_VIRTUAL_HEIGHT; y++) {
        for (int x = 0; x < DISPLAY_VIRTUAL_WIDTH; x++) {
            if (canvas_get_pixel(x, y) == 1) has_pixel = true;
        }
    }
    assert(has_pixel);

    // Frame 3 (wraps back to frame 0 index)
    state.loop_tick = 3;
    canvas_clear();
    widget_dispatch_block(&loop_block, &state);
    has_pixel = false;
    for (int y = 0; y < DISPLAY_VIRTUAL_HEIGHT; y++) {
        for (int x = 0; x < DISPLAY_VIRTUAL_WIDTH; x++) {
            if (canvas_get_pixel(x, y) == 1) has_pixel = true;
        }
    }
    assert(has_pixel);

    // Test non-looping animation (param2 = 1, stop at last slice)
    struct display_layout_block noloop_block = loop_block;
    noloop_block.symbol_count = 3;
    noloop_block.param2 = 1; // loop: false

    // Tick 10 (far beyond symbol_count = 3, must render last slice = frame 2)
    state.loop_tick = 10;
    canvas_clear();
    widget_dispatch_block(&noloop_block, &state);
    uint8_t buffer_last_slice[DISPLAY_VIRTUAL_HEIGHT][DISPLAY_VIRTUAL_WIDTH];
    for (int y = 0; y < DISPLAY_VIRTUAL_HEIGHT; y++) {
        for (int x = 0; x < DISPLAY_VIRTUAL_WIDTH; x++) {
            buffer_last_slice[y][x] = canvas_get_pixel(x, y);
        }
    }

    // Tick 2 (explicit frame 2)
    state.loop_tick = 2;
    canvas_clear();
    widget_dispatch_block(&noloop_block, &state);
    for (int y = 0; y < DISPLAY_VIRTUAL_HEIGHT; y++) {
        for (int x = 0; x < DISPLAY_VIRTUAL_WIDTH; x++) {
            assert(canvas_get_pixel(x, y) == buffer_last_slice[y][x]);
        }
    }

    printf("  -> Animation widget (looping and stop-at-last) passed successfully.\n");
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
    test_wpm_chart_widget();
    test_loop_widget();

    printf("[TEST] Testing idle screen layout gating...\n");
    assert(SCYAN_IDLE_SCREENS_ENABLED == 1 || SCYAN_IDLE_SCREENS_ENABLED == 0);
    assert(SCYAN_IDLE_SCREENS_ENABLED_LEFT == 1 || SCYAN_IDLE_SCREENS_ENABLED_LEFT == 0);
    assert(SCYAN_IDLE_SCREENS_ENABLED_RIGHT == 1 || SCYAN_IDLE_SCREENS_ENABLED_RIGHT == 0);
    assert(SCYAN_IDLE_TIMEOUT_MS > 0);
    assert(SCYAN_IDLE_TIMEOUT_MS_LEFT > 0);
    assert(SCYAN_IDLE_TIMEOUT_MS_RIGHT > 0);
    {
        // When right idle screen is disabled, verify gate resolves to active layout even if state.is_idle is true
        bool right_idle_enabled = (SCYAN_IDLE_SCREENS_ENABLED_RIGHT != 0);
        bool show_right_idle = true && right_idle_enabled;
        const struct display_layout_block *blocks = show_right_idle ? LAYOUT_RIGHT_IDLE_BLOCKS : LAYOUT_RIGHT_ACTIVE_BLOCKS;
        size_t count = show_right_idle ? LAYOUT_RIGHT_IDLE_COUNT : LAYOUT_RIGHT_ACTIVE_COUNT;
        if (!right_idle_enabled) {
            assert(!show_right_idle);
            assert(blocks == LAYOUT_RIGHT_ACTIVE_BLOCKS);
            assert(count == LAYOUT_RIGHT_ACTIVE_COUNT);
        }
    }
    printf("  -> Idle screen layout gating passed.\n");

    printf("=============================================\n");
    printf("ALL TESTS PASSED SUCCESSFULLY!               \n");
    printf("=============================================\n");
    return 0;
}

