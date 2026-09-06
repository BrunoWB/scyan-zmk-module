# zmk-display-core

A modular custom status screen library for vertical OLED displays (128x32 SSD1306) on ZMK keyboards (such as the Corne / CRKBD).

Features custom 1-bit pixel art, an event-driven architecture, battery gauge, WPM speed meter, split peripheral link monitor, and an automatic idle screen with customizable branding.

---

## Features

- **Dual Mode (Active & Idle)**: Automatically switches from the active typing screen to an idle screen after a configurable timeout (default: 10s).
- **Custom Pixel Art & Typography**:
  - Custom 5x5 uppercase pixel font & 8x10 digit font.
  - Multi-layer angle art & layer indicator brackets.
  - Dynamic 7-stage arrow WPM progress meter.
- **Configurable Idle Branding**: Customize the text shown on the idle screen via Kconfig (e.g. `CONFIG_CUSTOM_STATUS_SCREEN_USER_NAME="CORNE"`).
- **Split Role Awareness**:
  - **Central (Left)**: Full status screen (battery, USB/BLE endpoint, profile letter, layer, WPM, split link).
  - **Peripheral (Right)**: Battery level and split connection status.
- **Screen Rotation & Polarity**: Native 90° or 270° vertical rotation and pixel color inversion for SSD1306.

---

## Installation

Add this module to your ZMK user repository (`config/west.yml`):

```yaml
manifest:
  remotes:
    - name: zmkfirmware
      url-base: https://github.com/zmkfirmware
    - name: brunowb
      url-base: https://github.com/BrunoWB
  projects:
    - name: zmk
      remote: zmkfirmware
      revision: v0.3
      import: app/west.yml
    - name: zmk-display-core
      remote: brunowb
      revision: main
  self:
    path: config
```

---

## Configuration

Add the following to your `config/corne.conf` (or your shield's `.conf` file):

```ini
# Enable display and custom status screen
CONFIG_ZMK_DISPLAY=y
CONFIG_SSD1306=y
CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM=y
CONFIG_ZMK_DISPLAY_STATUS_SCREEN_BUILT_IN=n
CONFIG_LV_USE_CANVAS=y
CONFIG_LV_USE_IMG=y

# Orientation and color
CONFIG_CUSTOM_STATUS_SCREEN_ROTATION_90=y
CONFIG_CUSTOM_STATUS_SCREEN_ROTATION_270=n
CONFIG_CUSTOM_STATUS_SCREEN_INVERT=y

# Idle timeout (in milliseconds) and custom idle text
CONFIG_CUSTOM_STATUS_SCREEN_IDLE_TIMEOUT_MS=10000
CONFIG_CUSTOM_STATUS_SCREEN_USER_NAME="CORNE"
```

---

## Configuration Options

| Option | Type | Default | Description |
| :--- | :---: | :---: | :--- |
| `CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM` | bool | `n` | Enable this custom status screen widget. |
| `CONFIG_CUSTOM_STATUS_SCREEN_ROTATION_90` | bool | `y` | 90° vertical rotation (standard Corne mounting). |
| `CONFIG_CUSTOM_STATUS_SCREEN_ROTATION_270` | bool | `n` | 270° inverted vertical rotation. |
| `CONFIG_CUSTOM_STATUS_SCREEN_INVERT` | bool | `y` | Invert monochrome pixels (swaps black/white). |
| `CONFIG_CUSTOM_STATUS_SCREEN_IDLE_TIMEOUT_MS` | int | `10000` | Inactivity duration in ms before entering idle screen. |
| `CONFIG_CUSTOM_STATUS_SCREEN_USER_NAME` | string | `"SCYAN"` | Text rendered on the idle screen (up to 6 chars). |

---

## Online Editor & Customization

Design your own pixel art and preview layouts in your browser with the companion editor:
**[brunowb.github.io/zmk-display-builder/](https://brunowb.github.io/zmk-display-builder/)**
