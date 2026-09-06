import sys, os

def parse_bmp(bmp_path):
    with open(bmp_path, 'rb') as f:
        data = f.read()

    offset = int.from_bytes(data[10:14], 'little')
    width = int.from_bytes(data[18:22], 'little')
    height = int.from_bytes(data[22:26], 'little')

    grid = []
    for y in range(height):
        row = []
        row_offset = offset + (height - 1 - y) * width * 4
        for x in range(width):
            b = data[row_offset + x*4]
            g = data[row_offset + x*4 + 1]
            r = data[row_offset + x*4 + 2]
            lum = 0.299*r + 0.587*g + 0.114*b
            if lum < 50:
                row.append(0) # display off (black)
            elif lum > 200:
                row.append(1) # pixel on (white)
            else:
                row.append(2) # background/gray
        grid.append(row)
    return grid, width, height

bmp_path = 'assets/ref.bmp' if os.path.exists('assets/ref.bmp') else '/tmp/ref.bmp'
grid, img_w, img_h = parse_bmp(bmp_path)

# Extract Panel 1 (x=37..68, y=4..131)
# Extract Panel 2 (x=86..117, y=4..131)
# Extract Panel 3 (x=135..166, y=4..131)
panels = {}
for p_idx, (p_name, x0) in enumerate([('panel1', 37), ('panel2', 86), ('panel3', 135)]):
    p_data = []
    for y in range(4, 4+128):
        row = [grid[y][x0 + x] for x in range(32)]
        p_data.append(row)
    panels[p_name] = p_data

# Extract Skull for 4 layers:
# Width 26, height 23 (cols 88..113)
layer_skulls = {}
layer_skull_y = {
    0: 151,
    1: 215,
    2: 184,
    3: 248
}
for l_idx, y0 in layer_skull_y.items():
    skull_x = 88
    skull_rows = []
    for dy in range(23):
        y = y0 + dy
        row = [grid[y][skull_x + x] for x in range(26)]
        skull_rows.append(row)
    layer_skulls[l_idx] = skull_rows

# Extract Brackets for 4 layers:
layer_brackets = {}
for l_idx, y0 in [(0, 158), (1, 222), (2, 190), (3, 255)]:
    br_x0 = 45
    bracket_rows = []
    for dy in range(11):
        y = y0 + dy
        row = [grid[y][br_x0 + x] for x in range(22)]
        bracket_rows.append(row)
    layer_brackets[l_idx] = bracket_rows

# Extract Digits 0-9:
# Grid cells in bottom reference: each digit is 8 pixels wide
digits_cols = [
    (179, 186), # 0
    (188, 195), # 1
    (197, 204), # 2
    (207, 214), # 3
    (217, 224), # 4
    (227, 234), # 5
    (237, 244), # 6
    (247, 254), # 7
    (257, 264), # 8
    (267, 274), # 9
]
digits_data = []
for d_idx, (c0, c1) in enumerate(digits_cols):
    d_rows = []
    for y in range(244, 254):
        row = [grid[y][x] for x in range(c0, c1+1)]
        d_rows.append(row)
    digits_data.append(d_rows)

# Extract Alphabet A-Z:
def find_letters(y0, y1, x_start, x_end):
    cols_with_pixels = []
    for x in range(x_start, x_end):
        has_pixel = any(grid[y][x] == 1 for y in range(y0, y1+1))
        cols_with_pixels.append((x, has_pixel))
    letters = []
    cur_start = None
    for x, has_pixel in cols_with_pixels:
        if has_pixel:
            if cur_start is None:
                cur_start = x
        else:
            if cur_start is not None:
                letters.append((cur_start, x - 1))
                cur_start = None
    if cur_start is not None:
        letters.append((cur_start, x_end - 1))
    return letters

l1 = find_letters(188, 192, 180, 291) # A-T
l2 = find_letters(196, 200, 180, 260) # U-Z
all_letters_bounds = [(c0, c1, 188) for c0, c1 in l1] + [(c0, c1, 196) for c0, c1 in l2]

alphabet_data = {}
alphabet = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'
for i, (c0, c1, y0) in enumerate(all_letters_bounds):
    char = alphabet[i]
    w = c1 - c0 + 1
    rows = []
    for dy in range(5):
        rows.append([grid[y0 + dy][x] for x in range(c0, c1+1)])
    alphabet_data[char] = (w, rows)

# Extract Bluetooth icon (8x8):
bt_rows = []
for y in range(186, 194):
    bt_rows.append([grid[y][x] for x in range(172, 180)])

# Extract USB icon (12x10):
usb_rows = []
for y in range(4, 4+10):
    usb_rows.append([grid[y][37 + x] for x in range(0, 12)])

# Extract Split Connected and Disconnected icons (13x9, cols 10..22):
split_conn_rows = []
split_disc_rows = []
for y in range(4+116, 4+125):
    split_conn_rows.append([grid[y][37 + x] for x in range(10, 23)])
    split_disc_rows.append([grid[y][86 + x] for x in range(10, 23)])

# Ensure src directory exists
os.makedirs('src', exist_ok=True)

# Generate custom_display_assets.h
with open('src/custom_display_assets.h', 'w') as f:
    f.write('/* Auto-generated pixel art assets for Corne vertical OLED display */\n')
    f.write('#pragma once\n\n')
    f.write('#include <stdint.h>\n#include <stdbool.h>\n\n')

    f.write('#define DISPLAY_VIRTUAL_WIDTH  32\n')
    f.write('#define DISPLAY_VIRTUAL_HEIGHT 128\n')
    f.write('#define DISPLAY_HW_WIDTH       128\n')
    f.write('#define DISPLAY_HW_HEIGHT      32\n\n')

    def write_bitmap_1bpp(name, rows):
        h = len(rows)
        w = len(rows[0])
        stride = (w + 7) // 8
        f.write(f'// {name}: {w}x{h}\n')
        f.write(f'static const uint8_t {name}[{h * stride}] = {{\n')
        for y, row in enumerate(rows):
            line_bytes = []
            for b in range(stride):
                byte_val = 0
                for bit in range(8):
                    col = b * 8 + bit
                    if col < w and row[col] == 1:
                        byte_val |= (1 << (7 - bit))
                line_bytes.append(f'0x{byte_val:02X}')
            comment = ''.join('#' if v == 1 else ' ' for v in row)
            f.write(f'    {", ".join(line_bytes)}, // {comment}\n')
        f.write('};\n\n')

    # Digits 0-9 (8x10 each)
    f.write('// Digits 0-9 (width 8, height 10)\n')
    f.write('static const uint8_t FONT_DIGITS[10][10] = {\n')
    for d_idx, d_rows in enumerate(digits_data):
        f.write(f'    // Digit {d_idx}\n    {{\n')
        for row in d_rows:
            byte_val = 0
            for bit in range(8):
                if row[bit] == 1:
                    byte_val |= (1 << (7 - bit))
            comment = ''.join('#' if v == 1 else ' ' for v in row)
            f.write(f'        0x{byte_val:02X}, // {comment}\n')
        f.write('    },\n')
    f.write('};\n\n')

    # Alphabet A-Z
    f.write('struct glyph_letter {\n    uint8_t width;\n    uint8_t data[5];\n};\n\n')
    f.write('static const struct glyph_letter FONT_LETTERS[26] = {\n')
    for char in alphabet:
        w, rows = alphabet_data[char]
        f.write(f'    // Letter {char} (width {w})\n    {{\n')
        f.write(f'        .width = {w},\n        .data = {{\n')
        for r in rows:
            byte_val = 0
            for bit in range(w):
                if r[bit] == 1:
                    byte_val |= (1 << (7 - bit))
            comment = ''.join('#' if v == 1 else ' ' for v in r)
            f.write(f'            0x{byte_val:02X}, // {comment}\n')
        f.write('        }\n    },\n')
    f.write('};\n\n')

    # Skulls: 4 states (width 26, height 23, 4 bytes stride)
    f.write('// Skull graphics for 4 layers (26x23, 4 bytes stride)\n')
    f.write('// Layer 0: Free (looking straight), Layer 1: RightHold (looking right),\n')
    f.write('// Layer 2: LeftHold (looking left),     Layer 3: SimmHold (cross-eyed)\n')
    f.write('static const uint8_t SKULL_LAYERS[4][23 * 4] = {\n')
    for l_idx in range(4):
        f.write(f'    // Layer {l_idx}\n    {{\n')
        for row in layer_skulls[l_idx]:
            line_bytes = []
            for b in range(4):
                byte_val = 0
                for bit in range(8):
                    col = b * 8 + bit
                    if col < len(row) and row[col] == 1:
                        byte_val |= (1 << (7 - bit))
                line_bytes.append(f'0x{byte_val:02X}')
            comment = ''.join('#' if v == 1 else ' ' for v in row)
            f.write(f'        {", ".join(line_bytes)}, // {comment}\n')
        f.write('    },\n')
    f.write('};\n\n')

    # Brackets: 4 states (width 22, height 11, stride 3 bytes)
    f.write('// Layer indicator brackets (width 22, height 11, stride 3 bytes)\n')
    f.write('static const uint8_t BRACKET_LAYERS[4][11 * 3] = {\n')
    for l_idx in range(4):
        f.write(f'    // Layer {l_idx}\n    {{\n')
        for row in layer_brackets[l_idx]:
            line_bytes = []
            for b in range(3):
                byte_val = 0
                for bit in range(8):
                    col = b * 8 + bit
                    if col < len(row) and row[col] == 1:
                        byte_val |= (1 << (7 - bit))
                line_bytes.append(f'0x{byte_val:02X}')
            comment = ''.join('#' if v == 1 else ' ' for v in row)
            f.write(f'        {", ".join(line_bytes)}, // {comment}\n')
        f.write('    },\n')
    f.write('};\n\n')

    # Bluetooth icon (8x8)
    write_bitmap_1bpp('ICON_BLUETOOTH', bt_rows)

    # USB icon (12x10, stride 2)
    write_bitmap_1bpp('ICON_USB', usb_rows)

    # Split Connection icons (13x9, stride 2)
    write_bitmap_1bpp('ICON_SPLIT_CONNECTED', split_conn_rows)
    write_bitmap_1bpp('ICON_SPLIT_DISCONNECTED', split_disc_rows)

    # Full panels 1, 2, 3 as reference 32x128 1bpp buffers (disabled to save flash & prevent unused warnings)
    # for p_name in ['panel1', 'panel2', 'panel3']:
    #     write_bitmap_1bpp(f'PANEL_REF_{p_name.upper()}', panels[p_name])

print('Successfully generated src/custom_display_assets.h')
