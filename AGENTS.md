# Antigravity Operating Guide: Scyan ZMK Ecosystem

Persistent architectural reference for Antigravity pair programming in `scyan-zmk-module`.

---

## 1. Ecosystem Directory & Role Map

| Repository | Local Path (Host: `/var/home/Scyan/` == `/home/Scyan/`) | Stack | Role |
| :--- | :--- | :--- | :--- |
| **`scyan-zmk-module`** *(Here)* | `Projects/Firmware/scyan-zmk-module/` | Embedded C / Zephyr | Runtime 1bpp blitter, transform (90° rot), and widget engine. |
| **`zmk-config`** | `Projects/Firmware/zmk-config/` | West / Kconfig / CI | Corne split keyboard config; CI builds `.uf2` on asset push. |
| **`scyan-zmk-studio`** | `Projects/Web/scyan-zmk-studio/` | React 19 / TS 6 / Vite | Visual 2-Atlas IDE & layout compiler to C header. |
| **`bwpx-editor`** | `Projects/Web/bwpx-editor/` | React 19 / Vite | Upstream 1bpp pixel editor & raster algorithm core (`BwpxGrid`). |
| **`hello-web`** | `Projects/Web/hello-web/` | HTML/CSS | Public developer landing page & project portal. |

---

## 2. Unidirectional Data Pipeline

```
[scyan-zmk-studio] ──(generates)──> [scyan_assets.h in zmk-config]
                                                    │
                                                    ▼
                                    [scyan-zmk-module] (Here)
                                      ├── canvas.c (1bpp blitter)
                                      ├── transform.c (90°/270° rotation)
                                      ├── font_renderer.c (UTF-8 font engine)
                                      ├── events.c (ZMK event subscriptions)
                                      └── widgets/ (10 dynamic widget dispatchers)
```

* **Detailed Architecture Reference**: [`/home/Scyan/Projects/Web/scyan-zmk-studio/docs/ARCHITECTURE.md`](file:///home/Scyan/Projects/Web/scyan-zmk-studio/docs/ARCHITECTURE.md)
* **Hardware & Driver References**: [`/home/Scyan/Projects/References/zmk/`](file:///home/Scyan/Projects/References/zmk/) (offline ZMK docs in `zmk-doc/` — no web fetch needed)

---

## 3. The Core Contract: `include/scyan/types.h`

This header defines the exact contract between firmware and `scyan-zmk-studio`:
* **Strict Enum Sequence (`enum display_widget_type`)**:
  `1: OUTPUT_STATUS`, `2: BATTERY`, `3: LAYER`, `4: WPM`, `5: WPM_CHART`, `6: BRANDING`, `7: SPLIT`, `8: SCREENSAVER`, `9: CAPS_LOCK`, `10: BONGO`.
  *Never reorder or insert enums without synchronizing `src/services/cHeaderParser.ts` in `scyan-zmk-studio`.*
* **Layout Blocks (`struct display_layout_block`)**:
  Runtime dispatches `LAYOUT_LEFT_ACTIVE_BLOCKS`, `LAYOUT_LEFT_IDLE_BLOCKS`, `LAYOUT_RIGHT_ACTIVE_BLOCKS`, and `LAYOUT_RIGHT_IDLE_BLOCKS`.
* **Bitmaps & Strides**:
  Expects 1bpp MSB-first packing with stride `(width + 7) / 8`.

---

## 4. Non-Negotiable Invariants for Antigravity

1. **Contract Synchronization**:
   Any changes to struct layouts, block parameters (`param1`, `param2`, `param3`), or widget types in `include/scyan/types.h` MUST maintain 1:1 parity with `scyan-zmk-studio`.
2. **Dedicated Display Work Queue**:
   All screen rendering and event handlers must remain non-blocking, executing within the dedicated ZMK display thread (`CONFIG_ZMK_DISPLAY_WORK_QUEUE_DEDICATED=y`).
3. **90° Hardware Rotation Integrity**:
   Virtual vertical coordinates (32×128) are transformed to physical horizontal pages (128×32 SSD1306) via `src/transform.c`. Ensure rotation boundary calculations remain correct.
