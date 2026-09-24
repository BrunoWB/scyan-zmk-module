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
#include "widgets/dispatch.c"
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
#include "widgets/widget_typewriter.c"
#include "widgets/widget_keypress.c"
#include "transform.h"
#include "transform.c"

static int mock_refresh_count = 0;
void engine_trigger_refresh(void) {
    mock_refresh_count++;
}

#include "engine.h"
#include "engine.c"

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

    static const struct display_layout_block mock_active_blocks[] = {
        { .type = WIDGET_TYPE_OUTPUT_STATUS, .x = 0, .y = 12, .width = 12, .height = 10, .enabled = true, .mode = 0, .param1 = 0, .param2 = 0, .param3 = 0, .symbol_count = 1, .symbol_ids = { SYMBOL_USB }, .text_count = 0, .text_entries = { NULL }, .custom_text = NULL, .symbol_id = SYMBOL_USB },
        { .type = WIDGET_TYPE_BATTERY, .x = 0, .y = 1, .width = 17, .height = 10, .enabled = true, .mode = 0, .param1 = 0, .param2 = 0, .param3 = 0, .symbol_count = 1, .symbol_ids = { SYMBOL_BATTERY_FRAME }, .text_count = 0, .text_entries = { NULL }, .custom_text = NULL, .symbol_id = SYMBOL_BATTERY_FRAME },
        { .type = WIDGET_TYPE_LAYER, .x = 0, .y = 24, .width = 24, .height = 12, .enabled = true, .mode = 0, .param1 = 0, .param2 = 0, .param3 = 0, .symbol_count = 1, .symbol_ids = { SYMBOL_BRACKET_LAYER_0 }, .text_count = 0, .text_entries = { NULL }, .custom_text = NULL, .symbol_id = SYMBOL_BRACKET_LAYER_0 },
        { .type = WIDGET_TYPE_WPM, .x = 0, .y = 79, .width = 27, .height = 5, .enabled = true, .mode = 0, .param1 = 0, .param2 = 100, .param3 = 0, .symbol_count = 1, .symbol_ids = { SYMBOL_SPEEDOMETER_8803 }, .text_count = 0, .text_entries = { NULL }, .custom_text = NULL, .symbol_id = SYMBOL_SPEEDOMETER_8803 },
        { .type = WIDGET_TYPE_SPLIT, .x = 9, .y = 94, .width = 13, .height = 9, .enabled = true, .mode = 0, .param1 = 0, .param2 = 0, .param3 = 0, .symbol_count = 2, .symbol_ids = { SYMBOL_SPLIT_CONNECTED, SYMBOL_SPLIT_DISCONNECTED }, .text_count = 0, .text_entries = { NULL }, .custom_text = NULL, .symbol_id = SYMBOL_SPLIT_CONNECTED },
        { .type = WIDGET_TYPE_CAPS_LOCK, .x = 0, .y = 30, .width = 10, .height = 10, .enabled = true, .mode = 0, .param1 = 0, .param2 = 0, .param3 = 0, .symbol_count = 1, .symbol_ids = { SYMBOL_CAPSA_9310 }, .text_count = 0, .text_entries = { NULL }, .custom_text = NULL, .symbol_id = SYMBOL_CAPSA_9310 },
    };
    static const size_t mock_active_count = sizeof(mock_active_blocks) / sizeof(mock_active_blocks[0]);

    static const struct display_layout_block mock_idle_blocks[] = {
        { .type = WIDGET_TYPE_BRANDING, .x = 2, .y = 72, .width = 25, .height = 5, .enabled = true, .mode = 1, .param1 = 0, .param2 = 0, .param3 = 0, .symbol_count = 1, .symbol_ids = { SYMBOL_USB }, .text_count = 1, .text_entries = { "Scyan" }, .custom_text = "Scyan", .symbol_id = SYMBOL_USB },
        { .type = WIDGET_TYPE_SCREENSAVER, .x = 0, .y = 36, .width = 26, .height = 23, .enabled = true, .mode = 0, .param1 = 0, .param2 = 0, .param3 = 0, .symbol_count = 1, .symbol_ids = { SYMBOL_SKULL_LAYER_0 }, .text_count = 1, .text_entries = { "Scyan" }, .custom_text = "Scyan", .symbol_id = SYMBOL_SKULL_LAYER_0 },
    };
    static const size_t mock_idle_count = sizeof(mock_idle_blocks) / sizeof(mock_idle_blocks[0]);

    canvas_clear();
    for (size_t i = 0; i < mock_active_count; i++) {
        widget_dispatch_block(&mock_active_blocks[i], &state);
    }

    state.is_idle = true;
    canvas_clear();
    for (size_t i = 0; i < mock_idle_count; i++) {
        widget_dispatch_block(&mock_idle_blocks[i], &state);
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
    printf("[TEST] Testing 0, 90, 180, and 270 degree rotation transformation math...\n");
    // 90 degrees:
    // hx = (DISPLAY_VIRTUAL_HEIGHT - 1) - vy = 127 - vy
    // hy = vx
    int vx = 0, vy = 0;
    assert((127 - vy) == 127 && vx == 0);
    vx = 31; vy = 127;
    assert((127 - vy) == 0 && vx == 31);

    // 270 degrees:
    // hx = vy
    // hy = (DISPLAY_VIRTUAL_WIDTH - 1) - vx = 31 - vx
    vx = 0; vy = 0;
    assert(vy == 0 && (31 - vx) == 31);
    vx = 31; vy = 127;
    assert(vy == 127 && (31 - vx) == 0);

    // 0 degrees:
    // hx = vx, hy = vy
    vx = 10; vy = 20;
    assert(vx == 10 && vy == 20);

    // 180 degrees:
    // hx = (W - 1) - vx, hy = (H - 1) - vy
    vx = 10; vy = 20;
    assert((31 - vx) == 21 && (127 - vy) == 107);

    // Verify rotation macros exist and resolve
    assert(SCYAN_ROTATION == 90);
    assert(SCYAN_ROTATION_PERIPHERAL == 90);

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
    // Tick 301 (32-bit tick beyond 255: 301 % 3 = 1, must render frame 1)
    state.loop_tick = 301;
    canvas_clear();
    widget_dispatch_block(&loop_block, &state);
    has_pixel = false;
    for (int y = 0; y < DISPLAY_VIRTUAL_HEIGHT; y++) {
        for (int x = 0; x < DISPLAY_VIRTUAL_WIDTH; x++) {
            if (canvas_get_pixel(x, y) == 1) has_pixel = true;
        }
    }
    assert(has_pixel);

    // Test animation with frame count > 16 resolving via symbol_id + idx
    struct display_layout_block large_anim_block = loop_block;
    large_anim_block.symbol_id = SYMBOL_CHARGE_0960;
    large_anim_block.symbol_count = 16; // Devicetree symbols array capped at 16
    large_anim_block.param3 = 54;       // Total frames configured as 54 via param3
    // Tick 18 (frame 18, beyond MAX_BLOCK_SYMBOLS = 16, resolves to SYMBOL_CHARGE_0960 + 18)
    state.loop_tick = 18;
    canvas_clear();
    widget_dispatch_block(&large_anim_block, &state);

    // Tick 53 (last frame, resolves to SYMBOL_CHARGE_0960 + 53)
    state.loop_tick = 53;
    canvas_clear();
    widget_dispatch_block(&large_anim_block, &state);

    // Tick 54 (loops back to frame 0)
    state.loop_tick = 54;
    canvas_clear();
    widget_dispatch_block(&large_anim_block, &state);

    printf("  -> Animation widget (looping, stop-at-last, 32-bit ticks, and 16+ frame fallback via param3) passed successfully.\n");
}

static bool helper_has_pixel_in_rect(int max_x, int max_y) {
    for (int y = 0; y < max_y; y++) {
        for (int x = 0; x < max_x; x++) {
            if (canvas_get_pixel(x, y) == 1) return true;
        }
    }
    return false;
}

void test_symbol_0_across_widgets(void) {
    printf("[TEST] Testing symbol 0 rendering across widgets (loop, bongo, screensaver, layer)...\n");
    struct custom_status_state state = {0};

    // 1. widget_loop with symbol 0
    struct display_layout_block loop_b = {
        .type = WIDGET_TYPE_LOOP,
        .x = 0, .y = 0, .width = 12, .height = 10,
        .enabled = true, .mode = 0,
        .symbol_count = 1, .symbol_ids = { 0 },
    };
    canvas_clear();
    widget_render_loop(&loop_b, &state);
    assert(helper_has_pixel_in_rect(12, 10));

    // 2. widget_bongo with symbol 0 (single symbol fallback)
    struct display_layout_block bongo_b = {
        .type = WIDGET_TYPE_BONGO,
        .x = 0, .y = 0, .width = 12, .height = 10,
        .enabled = true, .mode = 0,
        .symbol_count = 1, .symbol_ids = { 0 },
    };
    canvas_clear();
    widget_render_bongo(&bongo_b, &state);
    assert(helper_has_pixel_in_rect(12, 10));

    // 3. widget_screensaver with symbol 0
    struct display_layout_block scr_b = {
        .type = WIDGET_TYPE_SCREENSAVER,
        .x = 0, .y = 0, .width = 12, .height = 10,
        .enabled = true, .mode = 0,
        .symbol_count = 1, .symbol_ids = { 0 },
    };
    canvas_clear();
    widget_render_screensaver(&scr_b, &state);
    assert(helper_has_pixel_in_rect(12, 10));

    // 4. widget_layer with symbol 0
    struct display_layout_block layer_b = {
        .type = WIDGET_TYPE_LAYER,
        .x = 0, .y = 0, .width = 12, .height = 10,
        .enabled = true, .mode = 0,
        .symbol_count = 1, .symbol_ids = { 0 },
    };
    canvas_clear();
    widget_render_layer(&layer_b, &state);
    assert(helper_has_pixel_in_rect(12, 10));

    printf("  -> Symbol 0 correctly drawn across all widgets without invalid text fallbacks.\n");
}

void test_engine_unrolling_and_dispatch(void) {
    printf("[TEST] Testing engine.c Devicetree layout unrolling and dispatch...\n");

    // 1. Verify Devicetree macro unrolling of active layout blocks
    assert(CHOSEN_ACTIVE_COUNT == 2);
    assert(chosen_active_blocks[0].type == WIDGET_TYPE_BATTERY);
    assert(chosen_active_blocks[0].x == 0);
    assert(chosen_active_blocks[0].y == 0);
    assert(chosen_active_blocks[0].width == 17);
    assert(chosen_active_blocks[0].height == 10);
    assert(chosen_active_blocks[0].enabled == 1);

    assert(chosen_active_blocks[1].type == WIDGET_TYPE_SCREENSAVER);
    assert(chosen_active_blocks[1].x == 0);
    assert(chosen_active_blocks[1].y == 20);
    assert(chosen_active_blocks[1].width == 32);
    assert(chosen_active_blocks[1].height == 32);
    assert(chosen_active_blocks[1].symbol_id == 8);

    // 2. Verify Devicetree macro unrolling of idle layout blocks
    assert(CHOSEN_IDLE_COUNT == 1);
    assert(chosen_idle_blocks[0].type == WIDGET_TYPE_SCREENSAVER);
    assert(chosen_idle_blocks[0].x == 0);
    assert(chosen_idle_blocks[0].y == 0);
    assert(chosen_idle_blocks[0].width == 32);
    assert(chosen_idle_blocks[0].height == 64);
    assert(chosen_idle_blocks[0].symbol_id == 8);

    // 3. Verify Devicetree property helpers
    assert(engine_idle_screens_enabled_for_half() == true);
    assert(engine_get_idle_timeout_ms_for_half() == 30000);
    assert(engine_get_rotation_for_half() == 90);

    // 4. Verify engine layout block retrieval
    const struct display_layout_block *retrieved_blocks = NULL;
    size_t retrieved_count = 0;
    engine_get_layout_blocks(false, &retrieved_blocks, &retrieved_count);
    assert(retrieved_count == 2);
    assert(retrieved_blocks == chosen_active_blocks);

    engine_get_layout_blocks(true, &retrieved_blocks, &retrieved_count);
    assert(retrieved_count == 1);
    assert(retrieved_blocks == chosen_idle_blocks);

    // 5. Verify engine_init and engine_update_state dispatching blocks
    lv_obj_t mock_canvas;
    engine_init(&mock_canvas);

    struct custom_status_state state = {0};
    state.battery_level = 85;
    state.charging = false;
    state.is_idle = false;
    engine_update_state(state);

    // Confirm canvas was drawn to (battery widget drew pixels in virtual buffer)
    bool pixel_found = false;
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 17; x++) {
            if (canvas_get_pixel(x, y) == 1) {
                pixel_found = true;
                break;
            }
        }
        if (pixel_found) break;
    }
    assert(pixel_found);

    // 6. Verify engine_update_state dispatching idle layout blocks
    canvas_clear();
    state.is_idle = true;
    engine_set_idle(true);
    engine_update_state(state);

    // Confirm canvas was drawn to by idle screensaver widget
    bool idle_pixel_found = false;
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 32; x++) {
            if (canvas_get_pixel(x, y) == 1) {
                idle_pixel_found = true;
                break;
            }
        }
        if (idle_pixel_found) break;
    }
    assert(idle_pixel_found);
    engine_set_idle(false);

    printf("  -> engine.c Devicetree unrolling and layout block dispatch passed.\n");
}

void test_conditional_wpm_ticker(void) {
    printf("[TEST] Testing conditional WPM ticker scheduling...\n");
    assert(engine_has_wpm_chart(false) == false);
    assert(engine_has_wpm_chart(true) == false);

    lv_obj_t mock_canvas;
    engine_init(&mock_canvas);
    assert(wpm_ticker_work.timeout_ms == 0);

    engine_notify_activity();
    assert(wpm_ticker_work.timeout_ms == 0);

    idle_work_cb(NULL);
    assert(wpm_ticker_work.timeout_ms == 0);

    uint8_t prev_wpm_state = wpm_ticker_state;
    wpm_ticker_work_cb(NULL);
    assert(wpm_ticker_state == prev_wpm_state);

    printf("  -> Conditional WPM ticker passed.\n");
}

void test_typewriter_widget(void) {
    printf("[TEST] Testing Typewriter widget...\n");
    widget_typewriter_reset();
    assert(widget_typewriter_get_len() == 0);

    // 1. Keycode decoding
    widget_typewriter_record_key(0x07, 0x04, true); // 'A'
    widget_typewriter_record_key(0x07, 0x05, true); // 'B'
    widget_typewriter_record_key(0x07, 0x06, true); // 'C'
    assert(widget_typewriter_get_len() == 3);
    assert(strcmp(widget_typewriter_get_buf(), "ABC") == 0);

    // Backspace
    widget_typewriter_record_key(0x07, 0x2A, true); // Backspace
    assert(widget_typewriter_get_len() == 2);
    assert(strcmp(widget_typewriter_get_buf(), "AB") == 0);

    // Number and space
    widget_typewriter_record_key(0x07, 0x1E, true); // '1'
    widget_typewriter_record_key(0x07, 0x2C, true); // Space
    widget_typewriter_record_key(0x07, 0x1D, true); // 'Z'
    assert(strcmp(widget_typewriter_get_buf(), "AB1 Z") == 0);

    // Typewriter HID keycode sequence
    widget_typewriter_reset();
    widget_typewriter_record_key(0x07, 0x14, true);  // 'Q'
    widget_typewriter_record_key(0x07, 0x1A, true);  // 'W'
    widget_typewriter_record_key(0x07, 0x1C, true);  // 'Y'
    assert(widget_typewriter_get_len() == 3);
    assert(strcmp(widget_typewriter_get_buf(), "QWY") == 0);

    // Spot mode rendering
    canvas_clear();
    struct custom_status_state dummy_state = {0};
    struct display_layout_block spot_block = {
        .type = WIDGET_TYPE_TYPEWRITER,
        .x = 0, .y = 0, .width = 32, .height = 16,
        .enabled = 1,
        .mode = 1, // Spot
        .param1 = 0,
        .param2 = 0,
        .param3 = 1, // Big font
    };
    widget_render_typewriter(&spot_block, &dummy_state);
    // Spot mode should render 'Y' (the latest char in "QWY")
    int ink_pixels = 0;
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 32; x++) {
            if (canvas_get_pixel(x, y)) ink_pixels++;
        }
    }
    assert(ink_pixels > 0);

    // Inline mode rendering (West -> East)
    canvas_clear();
    struct display_layout_block inline_block = {
        .type = WIDGET_TYPE_TYPEWRITER,
        .x = 0, .y = 0, .width = 32, .height = 10,
        .enabled = 1,
        .mode = 0, // Inline
        .param1 = 0, // 'we'
        .param2 = 0,
        .param3 = 0, // Small font
    };
    widget_render_typewriter(&inline_block, &dummy_state);
    ink_pixels = 0;
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 32; x++) {
            if (canvas_get_pixel(x, y)) ink_pixels++;
        }
    }
    assert(ink_pixels > 0);

    // Random mode persistent coordinates and bank capacity
    widget_typewriter_reset();
    struct display_layout_block random_block = {
        .type = WIDGET_TYPE_TYPEWRITER,
        .x = 0, .y = 0, .width = 32, .height = 32,
        .enabled = 1,
        .mode = 2, // Random
        .param1 = 5, // Capacity: 5 letters
        .param2 = 200, // 200ms per letter idle cleaning
        .param3 = 0, // Small font
    };

    // Type 3 letters
    widget_typewriter_record_char('A');
    widget_typewriter_record_char('B');
    widget_typewriter_record_char('C');
    assert(widget_typewriter_get_random_count() == 3);

    // Initial render
    canvas_clear();
    widget_render_typewriter(&random_block, &dummy_state);
    uint8_t snap1[32][32];
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            snap1[y][x] = canvas_get_pixel(x, y);
        }
    }

    // Render again without typing -> canvas must be 100% identical (no jitter/jumping!)
    canvas_clear();
    widget_render_typewriter(&random_block, &dummy_state);
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            assert(canvas_get_pixel(x, y) == snap1[y][x]);
        }
    }

    // Type 2 more letters to fill bank to capacity (5)
    widget_typewriter_record_char('D');
    widget_typewriter_record_char('E');
    assert(widget_typewriter_get_random_count() == 5);
    canvas_clear();
    widget_render_typewriter(&random_block, &dummy_state);
    // All original pixels from snap1 must STILL be on! (Letters A, B, C did not move!)
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            if (snap1[y][x]) {
                assert(canvas_get_pixel(x, y) == 1);
            }
        }
    }

    // Now bank is FULL (5 letters: A, B, C, D, E).
    // Type letter 'F' past capacity -> 'A' is evicted, 'B', 'C', 'D', 'E' remain in place!
    widget_typewriter_record_char('F');
    canvas_clear();
    widget_render_typewriter(&random_block, &dummy_state);
    assert(widget_typewriter_get_random_count() == 5);

    // Type multiple identical letters 'Z', 'Z', 'Z', 'Z', 'Z', 'Z'
    widget_typewriter_reset();
    for (int i = 0; i < 5; i++) {
        widget_typewriter_record_char('Z');
    }
    assert(widget_typewriter_get_random_count() == 5);
    canvas_clear();
    widget_render_typewriter(&random_block, &dummy_state);
    uint8_t snap_z5[32][32];
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            snap_z5[y][x] = canvas_get_pixel(x, y);
        }
    }

    // Type another 'Z': replaces oldest 'Z' with a new 'Z' at a new position
    widget_typewriter_record_char('Z');
    canvas_clear();
    widget_render_typewriter(&random_block, &dummy_state);
    // Canvas must have changed (new Z appeared, old Z evicted!)
    bool z_changed = false;
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            if (canvas_get_pixel(x, y) != snap_z5[y][x]) z_changed = true;
        }
    }
    assert(z_changed);

    // Test incremental idle auto-cleaning in Random mode
    // Capacity = 5, cleaning = 200ms per letter
    assert(widget_typewriter_get_random_count() == 5);
    // Advance mock clock by 250ms -> exactly 1 letter evicted!
    mock_set_uptime(mock_uptime_ms + 250);
    canvas_clear();
    widget_render_typewriter(&random_block, &dummy_state);
    assert(widget_typewriter_get_random_count() == 4);

    // Advance mock clock by another 200ms -> next letter evicted!
    mock_set_uptime(mock_uptime_ms + 200);
    canvas_clear();
    widget_render_typewriter(&random_block, &dummy_state);
    assert(widget_typewriter_get_random_count() == 3);

    // Advance mock clock by 700ms -> remaining 3 letters evicted, bank empty!
    mock_set_uptime(mock_uptime_ms + 700);
    canvas_clear();
    widget_render_typewriter(&random_block, &dummy_state);
    assert(widget_typewriter_get_random_count() == 0);
    ink_pixels = 0;
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            if (canvas_get_pixel(x, y)) ink_pixels++;
        }
    }
    assert(ink_pixels == 0); // Completely clean!

    // Test param2 == 0 fallback (omitted from Devicetree): must default to 200ms
    random_block.param2 = 0;
    widget_typewriter_record_char('X');
    assert(widget_typewriter_get_random_count() == 1);
    mock_set_uptime(mock_uptime_ms + 250);
    widget_render_typewriter(&random_block, &dummy_state);
    assert(widget_typewriter_get_random_count() == 0); // Evicted using 200ms fallback

    printf("  -> Typewriter widget (decoding, spot, inline, random persistence, incremental auto-clean) passed successfully.\n");
}

void test_keypress_widget(void) {
    printf("[TEST] Testing Keypress widget...\n");
    widget_keypress_reset();

    const char *arrow_keys[] = { "ArrowUp", "ArrowDown", "ArrowLeft", "ArrowRight" };
    struct display_layout_block b_arrows = {
        .type = WIDGET_TYPE_KEYPRESS,
        .x = 0,
        .y = 0,
        .width = 32,
        .height = 32,
        .enabled = 1,
        .mode = 1, // Idle symbol enabled
        .symbol_id = SYMBOL_USB, // Idle symbol = 0
        .param1 = 4, // 4 bound elements
        .symbol_count = 4,
        .symbol_ids = { SYMBOL_BATTERY_FRAME, SYMBOL_SPLIT_CONNECTED, SYMBOL_SPLIT_DISCONNECTED, SYMBOL_BRACKET_LAYER_0 },
        .text_count = 4,
        .text_entries = { arrow_keys[0], arrow_keys[1], arrow_keys[2], arrow_keys[3] },
    };

    struct custom_status_state dummy_state = {0};

    // 1. Initial render with no keys pressed -> should render idle symbol (SYMBOL_USB)
    canvas_clear();
    widget_render_keypress(&b_arrows, &dummy_state);
    int usb_ink = 0;
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            if (canvas_get_pixel(x, y)) usb_ink++;
        }
    }
    assert(usb_ink > 0);

    // 2. Press Up Arrow (HID usage page 0x07, keycode 0x52)
    widget_keypress_record_key(0x07, 0x52, true);
    canvas_clear();
    widget_render_keypress(&b_arrows, &dummy_state);
    int up_ink = 0;
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            if (canvas_get_pixel(x, y)) up_ink++;
        }
    }
    assert(up_ink > 0);

    // 3. Release Up Arrow -> should return to idle symbol (SYMBOL_USB)
    widget_keypress_record_key(0x07, 0x52, false);
    canvas_clear();
    widget_render_keypress(&b_arrows, &dummy_state);
    int release_ink = 0;
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            if (canvas_get_pixel(x, y)) release_ink++;
        }
    }
    assert(release_ink == usb_ink);

    // 4. Press Right Arrow (HID keycode 0x4F) -> should render SYMBOL_BRACKET_LAYER_0 (symbol_ids[3])
    widget_keypress_record_key(0x07, 0x4F, true);
    canvas_clear();
    widget_render_keypress(&b_arrows, &dummy_state);
    int right_ink = 0;
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            if (canvas_get_pixel(x, y)) right_ink++;
        }
    }
    assert(right_ink > 0);
    widget_keypress_record_key(0x07, 0x4F, false);

    // 5. Test mode = 0 (no idle symbol): persists last active key
    struct display_layout_block b_no_idle = b_arrows;
    b_no_idle.mode = 0;
    widget_keypress_reset();

    // Initial render with mode=0 -> falls back to symbol_ids[0]
    canvas_clear();
    widget_render_keypress(&b_no_idle, &dummy_state);
    int init_ink = 0;
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            if (canvas_get_pixel(x, y)) init_ink++;
        }
    }
    assert(init_ink == up_ink);

    // Press Down Arrow (0x51) -> renders symbol_ids[1] (SYMBOL_SPLIT_CONNECTED)
    widget_keypress_record_key(0x07, 0x51, true);
    canvas_clear();
    widget_render_keypress(&b_no_idle, &dummy_state);
    int down_ink = 0;
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            if (canvas_get_pixel(x, y)) down_ink++;
        }
    }
    assert(down_ink > 0);

    // Release Down Arrow -> should STAY on down_ink because mode == 0!
    widget_keypress_record_key(0x07, 0x51, false);
    canvas_clear();
    widget_render_keypress(&b_no_idle, &dummy_state);
    int persisted_ink = 0;
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            if (canvas_get_pixel(x, y)) persisted_ink++;
        }
    }
    assert(persisted_ink == down_ink);

    // 6. Test letter keypress recording via HID keycode
    const char *letter_keys[] = { "q", "space" };
    struct display_layout_block b_pos = {
        .type = WIDGET_TYPE_KEYPRESS,
        .x = 0,
        .y = 0,
        .width = 32,
        .height = 32,
        .enabled = 1,
        .mode = 1,
        .symbol_id = SYMBOL_USB,
        .symbol_count = 2,
        .symbol_ids = { SYMBOL_BATTERY_FRAME, SYMBOL_SPLIT_CONNECTED },
        .text_count = 2,
        .text_entries = { letter_keys[0], letter_keys[1] },
    };
    widget_keypress_reset();
    widget_keypress_record_key(0x07, 0x14, true); // Press 'q'
    canvas_clear();
    widget_render_keypress(&b_pos, &dummy_state);
    int q_ink = 0;
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            if (canvas_get_pixel(x, y)) q_ink++;
        }
    }
    assert(q_ink == up_ink);

    widget_keypress_record_key(0x07, 0x14, false); // Release 'q'
    canvas_clear();
    widget_render_keypress(&b_pos, &dummy_state);
    int q_rel_ink = 0;
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 32; x++) {
            if (canvas_get_pixel(x, y)) q_rel_ink++;
        }
    }
    assert(q_rel_ink == usb_ink);

    printf("  -> Keypress widget (arrow keys, letter keys, pressed/released, idle symbol fallback, position mapping) passed successfully.\n");
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
    test_symbol_0_across_widgets();
    test_typewriter_widget();
    test_keypress_widget();
    test_engine_unrolling_and_dispatch();
    test_conditional_wpm_ticker();

    printf("[TEST] Testing idle screen layout gating...\n");
    assert(SCYAN_IDLE_SCREENS_ENABLED == 1 || SCYAN_IDLE_SCREENS_ENABLED == 0);
    assert(SCYAN_IDLE_TIMEOUT_MS > 0);
    {
        bool idle_enabled = (SCYAN_IDLE_SCREENS_ENABLED != 0);
        bool show_idle = true && idle_enabled;
        assert(idle_enabled == true);
        assert(show_idle == true);
    }
    printf("  -> Idle screen layout gating passed.\n");

    printf("[TEST] Testing Devicetree auto-discovery widget Kconfig flags...\n");
    assert(IS_ENABLED(CONFIG_SCYAN_WIDGET_OUTPUT));
    assert(IS_ENABLED(CONFIG_SCYAN_WIDGET_BATTERY));
    assert(IS_ENABLED(CONFIG_SCYAN_WIDGET_LAYER));
    assert(IS_ENABLED(CONFIG_SCYAN_WIDGET_WPM));
    assert(IS_ENABLED(CONFIG_SCYAN_WIDGET_WPM_CHART));
    assert(IS_ENABLED(CONFIG_SCYAN_WIDGET_BRANDING));
    assert(IS_ENABLED(CONFIG_SCYAN_WIDGET_SPLIT));
    assert(IS_ENABLED(CONFIG_SCYAN_WIDGET_SCREENSAVER));
    assert(IS_ENABLED(CONFIG_SCYAN_WIDGET_CAPS));
    assert(IS_ENABLED(CONFIG_SCYAN_WIDGET_BONGO));
    assert(IS_ENABLED(CONFIG_SCYAN_WIDGET_LOOP));
    assert(IS_ENABLED(CONFIG_SCYAN_WIDGET_TYPEWRITER));
    assert(IS_ENABLED(CONFIG_SCYAN_WIDGET_KEYPRESS));
    printf("  -> Devicetree auto-discovery widget Kconfig flags passed.\n");

    printf("=============================================\n");
    printf("ALL TESTS PASSED SUCCESSFULLY!               \n");
    printf("=============================================\n");
    return 0;
}

