#ifndef RENDERER_H
#define RENDERER_H

#include <GLES3/gl3.h>
#include <stdint.h>

#define GRID_COLS 80
#define GRID_ROWS 24

typedef struct {
    uint32_t char_id;
    float x;
    float y;
    uint32_t fg_color;
    uint32_t bg_color;
} CellMetadata;

typedef struct {
    GLuint program;
    GLuint vao;
    GLuint vbo_quad;
    GLuint vbo_metadata;
    GLuint texture_atlas;
    CellMetadata screen_grid[GRID_ROWS * GRID_COLS];
} GlesRenderer;

void renderer_init(GlesRenderer* renderer);
void renderer_update_grid(GlesRenderer* renderer, const uint8_t* screen_matrix);
void renderer_render_frame(GlesRenderer* renderer);

#endif
