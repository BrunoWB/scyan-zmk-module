# Scyan ZMK Module (`brunowb/scyan-zmk-module`)

> **Dynamic display runtime engine and layout interpreter for custom ZMK keyboard firmware.**

[![ZMK Module](https://img.shields.io/badge/ZMK-Module-blue?style=flat-square)](https://zmk.dev/)
[![Zephyr RTOS](https://img.shields.io/badge/Zephyr-RTOS-green?style=flat-square)](https://zephyrproject.org/)

---

## 📖 Overview

`scyan-zmk-module` is an external Zephyr / ZMK module that acts as the display firmware runtime for split keyboards (such as the Corne / CRKBD with nice_nano_v2 and SSD1306 OLED displays).

It interprets 2-Atlas spritesheets, custom font glyphs, and dynamic screen layout blocks generated and exported by **[Scyan ZMK Studio](https://github.com/BrunoWB/scyan-zmk-studio)** directly from your `zmk-config` repository.

---

## 🔄 The 3-Repository Ecosystem

```
+---------------------------+       +---------------------------+       +---------------------------+
|  brunowb/scyan-zmk-studio | ----> |    brunowb/zmk-config     | <---- |  brunowb/scyan-zmk-module |
| (Interactive Web Layout UI|  git  | (User Keymap, Config & CI)|  West |   (Display Engine Module) |
| & 2-Atlas Sprite Editor)  | push  | scyan_assets.h            | Module|     [This Repository]     |
+---------------------------+       +---------------------------+       +---------------------------+
```

1. **[Scyan ZMK Studio](https://github.com/BrunoWB/scyan-zmk-studio)**: Visual web app to draw 1bpp monochrome icons, customize fonts, and organize active & idle widget blocks via drag-and-drop. It commits `config/scyan_assets.h` into your `zmk-config` repository.
2. **[Bruno's ZMK Config](https://github.com/BrunoWB/zmk-config)**: User configuration containing `corne.keymap`, `corne.conf`, `build.yaml`, and `config/west.yml`.
3. **[Scyan ZMK Module](https://github.com/BrunoWB/scyan-zmk-module) (This repo)**: The runtime engine included in `west.yml` that reads `scyan_assets.h` and handles display rendering, orientation transforms, and ZMK event orchestration.

---

## ✨ Features

- **🎨 2-Atlas Direct Blitting**: Fast 1bpp bitwise rasterizer for `SYMBOLS_ATLAS` (icons, frames, gauges) and `FONT_ATLAS` (custom fonts and glyphs).
- **🔤 Dynamic UTF-8 & Font Engine**: Variable-width character rendering supporting small, big, digits, and text font styles.
- **🧩 10 Dynamic Widget Types**:
  - `WIDGET_TYPE_OUTPUT_STATUS`: USB / BLE profile connection status.
  - `WIDGET_TYPE_BATTERY`: Discrete multi-state symbol bars or text % divisions.
  - `WIDGET_TYPE_LAYER`: Dynamic layer numbers, layer names, bracket frames, or skull art.
  - `WIDGET_TYPE_WPM`: Real-time typing speed numerical readout and speedometer gauges.
  - `WIDGET_TYPE_WPM_CHART`: Historical typing speed sparkline / histogram graph.
  - `WIDGET_TYPE_BRANDING`: Custom text labels or branding artwork.
  - `WIDGET_TYPE_SPLIT`: Split link interconnect status (chain icon or status text).
  - `WIDGET_TYPE_SCREENSAVER`: Low-power static idle artwork.
  - `WIDGET_TYPE_CAPS_LOCK`: Caps lock state indicator.
- **⚡ Zero Typing Lag**: Key events update activity timestamps without interrupting keyboard scanning; re-rendering is debounced onto ZMK's dedicated display thread.
- **🔄 Rotation & Transform Pipeline**: Supports 90° and 270° orientation transforms and monochrome color inversion.
- **🌗 Split Central & Peripheral Synchronization**: Automatically renders `LAYOUT_LEFT_*` blocks on the Central half and `LAYOUT_RIGHT_*` blocks on the Peripheral half.
- **⏱️ Active & Idle Screen Transition**: Automatically switches to idle blocks after a configurable timeout (default 10s) and wakes up immediately on the next keystroke.

---

## ⚙️ Kconfig Configuration Options

Add these options to your `corne.conf` (or target shield config) in `zmk-config`:

| Kconfig Symbol | Type | Default | Description |
|---|---|---|---|
| `CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM` | bool | `y` | Enables custom status screen provided by this module |
| `CONFIG_SCYAN_ROTATION_90` | bool | `y` | 90° rotation (standard Corne vertical OLED orientation) |
| `CONFIG_SCYAN_ROTATION_270` | bool | `n` | 270° rotation (inverted vertical orientation) |
| `CONFIG_SCYAN_ROTATION_0` | bool | `n` | 0° rotation (standard horizontal orientation) |
| `CONFIG_SCYAN_INVERT` | bool | `y` | Invert monochrome pixels (SSD1306 contrast optimization) |
| `CONFIG_SCYAN_IDLE_TIMEOUT_MS` | int | `10000` | Inactivity time in ms before switching to idle screen |
| `CONFIG_SCYAN_LEFT_IS_CENTRAL` | bool | `y` | Map Left screen blocks to Central split role |
| `CONFIG_SCYAN_USER_NAME` | string | `"SCYAN"`| Fallback text for branding widgets |
| `CONFIG_SCYAN_BONGO_TAP_MS` | int | `60` | Bongo Cat tap animation duration in milliseconds |

---

## 📁 Repository Structure

```
scyan-zmk-module/
├── zephyr/
│   └── module.yml                  # Zephyr / West module declaration
├── CMakeLists.txt                  # Priority include pathing & source compilation
├── Kconfig                         # Module configuration definitions
├── include/
│   └── scyan/
│       ├── display.h               # Public status screen entrypoint prototype
│       └── types.h                 # Core types, contracts, and fail-fast header guards
├── src/
│   ├── status_screen.c             # zmk_display_status_screen() implementation
│   ├── engine.c                    # State diffing, dirty checking, and work queue scheduler
│   ├── canvas.c                    # 1bpp virtual buffer (32x128) drawing primitives
│   ├── transform.c                 # Rotation (90°/270°) and color inversion transforms
│   ├── font_renderer.c             # UTF-8 parser and variable-width glyph rasterizer
│   ├── events.c                    # ZMK event listeners (Battery, USB, BLE, Layer, WPM, Split)
│   └── widgets/
│       ├── widgets.h               # Widget dispatch definitions
│       ├── widget_output.c         # Output status widget
│       ├── widget_battery.c        # Battery level widget
│       ├── widget_layer.c          # Active layer widget
│       ├── widget_wpm.c            # WPM readout and sparkline chart
│       ├── widget_branding.c       # Branding & custom text widget
│       ├── widget_split.c          # Split peripheral link widget
│       ├── widget_screensaver.c    # Idle screensaver widget
│       └── widget_caps.c           # Caps Lock widget
└── tests/
    ├── mock_zmk.h                  # Host test mocks
    └── test_scyan_module.c         # Unit test suite
```

---

## 🧪 Testing

The repository includes a host-executable test suite that validates the rasterizer, font rendering, widget dispatcher, and orientation math against generated layout headers:

```bash
gcc -Wall -Wextra -Werror -I. -Iinclude -Isrc -Itests -I../zmk-config/config tests/test_scyan_module.c -o tests/test_runner
./tests/test_runner
```

