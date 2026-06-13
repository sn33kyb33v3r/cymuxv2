#ifndef FONT_ATLAS_H
#define FONT_ATLAS_H

#include <stdint.h>

#define GLYPH_WIDTH  8
#define GLYPH_HEIGHT 16
#define FONT_CHAR_COUNT 256

extern const uint8_t terminal_font_bitmap[FONT_CHAR_COUNT * GLYPH_HEIGHT];

#endif
