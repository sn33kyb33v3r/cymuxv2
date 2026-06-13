#define _GNU_SOURCE
#include <stdint.h>
#include <string.h>
#include "cymux_engine.h"

void parse_ansi_stream(CymuxRuntime *runtime, const uint8_t *buf, size_t len) {
    for (size_t i = 0; i < len; i++) {
        uint8_t c = buf[i];
        
        if (c == 0x1B && (i + 1 < len) && buf[i+1] == '[') {
            i += 2;
            uint32_t param = 0;
            while (i < len && (buf[i] >= '0' && buf[i] <= ';')) {
                if (buf[i] >= '0' && buf[i] <= '9') {
                    param = param * 10 + (buf[i] - '0');
                }
                i++;
            }
            if (i < len && buf[i] == 'H') {
                runtime->cursor_row = 0;
                runtime->cursor_col = 0;
            } else if (i < len && buf[i] == 'J') {
                for (int idx = 0; idx < SCREEN_ROWS * SCREEN_COLS; idx++) {
                    runtime->matrix_layout.grid[idx].character = ' ';
                    runtime->matrix_layout.grid[idx].fg_color = 7;
                    runtime->matrix_layout.grid[idx].bg_color = 0;
                }
            }
            continue;
        }

        if (c == '\n') {
            runtime->cursor_row++;
            runtime->cursor_col = 0;
        } else if (c == '\r') {
            runtime->cursor_col = 0;
        } else if (c >= 32 && c < 127) {
            if (runtime->cursor_row >= SCREEN_ROWS) {
                memmove(runtime->matrix_layout.grid, 
                        runtime->matrix_layout.grid + SCREEN_COLS, 
                        (SCREEN_ROWS - 1) * SCREEN_COLS * sizeof(GridCell));
                for (int col = 0; col < SCREEN_COLS; col++) {
                    int idx = (SCREEN_ROWS - 1) * SCREEN_COLS + col;
                    runtime->matrix_layout.grid[idx].character = ' ';
                    runtime->matrix_layout.grid[idx].fg_color = 7;
                    runtime->matrix_layout.grid[idx].bg_color = 0;
                }
                runtime->cursor_row = SCREEN_ROWS - 1;
            }
            if (runtime->cursor_col < SCREEN_COLS) {
                int idx = runtime->cursor_row * SCREEN_COLS + runtime->cursor_col;
                runtime->matrix_layout.grid[idx].character = c;
                runtime->matrix_layout.grid[idx].fg_color = 7;
                runtime->matrix_layout.grid[idx].bg_color = 0;
                runtime->cursor_col++;
            }
        }
    }
    runtime->matrix.revision++;
}
