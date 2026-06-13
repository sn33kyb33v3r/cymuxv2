#include "../core/cymux_engine.h"
#include <arm_neon.h>
#include <string.h>

void parse_ansi_stream(const uint8_t* buffer, size_t len, ScreenMatrix* matrix) {
    size_t i = 0;
    static uint32_t cursor_pos = 0;
    static uint8_t current_fg = 7;
    static uint8_t current_bg = 0;

    while (i + 16 <= len) {
        uint8x16_t data = vld1q_u8(&buffer[i]);
        
        uint8x16_t esc_mask = vceqq_u8(data, vdupq_n_u8(0x1B));
        uint8x16_t cr_mask  = vceqq_u8(data, vdupq_n_u8(0x0D));
        uint8x16_t nl_mask  = vceqq_u8(data, vdupq_n_u8(0x0A));
        
        uint8x16_t control_mask = vorrq_u8(esc_mask, vorrq_u8(cr_mask, nl_mask));
        
        uint64x2_t neon_bits = vreinterpretq_u64_u8(control_mask);
        uint64_t low = vgetq_lane_u64(neon_bits, 0);
        uint64_t high = vgetq_lane_u64(neon_bits, 1);

        if ((low | high) == 0) {
            for (int j = 0; j < 16; ++j) {
                if (cursor_pos >= (SCREEN_ROWS * SCREEN_COLS)) {
                    memmove(matrix->grid, matrix->grid + SCREEN_COLS, (SCREEN_ROWS - 1) * SCREEN_COLS * sizeof(GridCell));
                    memset(matrix->grid + (SCREEN_ROWS - 1) * SCREEN_COLS, 0, SCREEN_COLS * sizeof(GridCell));
                    cursor_pos = (SCREEN_ROWS - 1) * SCREEN_COLS;
                }
                matrix->grid[cursor_pos].character = buffer[i + j];
                matrix->grid[cursor_pos].fg_color = current_fg;
                matrix->grid[cursor_pos].bg_color = current_bg;
                matrix->grid[cursor_pos].attrib = 0;
                cursor_pos++;
            }
            i += 16;
        } else {
            break;
        }
    }

    for (; i < len; ++i) {
        uint8_t c = buffer[i];

        if (c == 0x1B) {
            if (i + 2 < len && buffer[i + 1] == '[') {
                i += 2;
                uint32_t val = 0;
                while (i < len && buffer[i] >= '0' && buffer[i] <= '9') {
                    val = val * 10 + (buffer[i] - '0');
                    i++;
                }
                if (i < len && buffer[i] == 'm') {
                    if (val == 0) {
                        current_fg = 7;
                        current_bg = 0;
                    } else if (val >= 30 && val <= 37) {
                        current_fg = val - 30;
                    } else if (val >= 40 && val <= 47) {
                        current_bg = val - 40;
                    }
                }
            }
            continue;
        }

        if (c == '\r') {
            cursor_pos = (cursor_pos / SCREEN_COLS) * SCREEN_COLS;
            continue;
        }

        if (c == '\n') {
            cursor_pos = ((cursor_pos / SCREEN_COLS) + 1) * SCREEN_COLS;
            if (cursor_pos >= (SCREEN_ROWS * SCREEN_COLS)) {
                memmove(matrix->grid, matrix->grid + SCREEN_COLS, (SCREEN_ROWS - 1) * SCREEN_COLS * sizeof(GridCell));
                memset(matrix->grid + (SCREEN_ROWS - 1) * SCREEN_COLS, 0, SCREEN_COLS * sizeof(GridCell));
                cursor_pos = (SCREEN_ROWS - 1) * SCREEN_COLS;
            }
            continue;
        }

        if (cursor_pos >= (SCREEN_ROWS * SCREEN_COLS)) {
            memmove(matrix->grid, matrix->grid + SCREEN_COLS, (SCREEN_ROWS - 1) * SCREEN_COLS * sizeof(GridCell));
            memset(matrix->grid + (SCREEN_ROWS - 1) * SCREEN_COLS, 0, SCREEN_COLS * sizeof(GridCell));
            cursor_pos = (SCREEN_ROWS - 1) * SCREEN_COLS;
        }

        matrix->grid[cursor_pos].character = c;
        matrix->grid[cursor_pos].fg_color = current_fg;
        matrix->grid[cursor_pos].bg_color = current_bg;
        matrix->grid[cursor_pos].attrib = 0;
        cursor_pos++;
    }

    atomic_fetch_add_explicit(&matrix->revision, 1, memory_order_release);
}
