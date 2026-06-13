#define _GNU_SOURCE
#include "../core/cymux_engine.h"
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <stdlib.h>

typedef struct {
    GLuint program;
    GLuint vbo;
    GLuint vao;
    GLuint instance_vbo;
    GLuint texture_id;
    float metadata_buffer[SCREEN_ROWS * SCREEN_COLS * 4];
} GLRenderer;

static const float quad_vertices[] = {
    -1.0f / 80.0f,  1.0f / 22.5f, 0.0f, 0.0f,
    -1.0f / 80.0f, -1.0f / 22.5f, 0.0f, 1.0f,
     1.0f / 80.0f, -1.0f / 22.5f, 1.0f, 1.0f,
    -1.0f / 80.0f,  1.0f / 22.5f, 0.0f, 0.0f,
     1.0f / 80.0f, -1.0f / 22.5f, 1.0f, 1.0f,
     1.0f / 80.0f,  1.0f / 22.5f, 1.0f, 0.0f
};

void init_gles_pipeline(GLRenderer* state, const uint8_t* font_data) {
    glGenVertexArrays(1, &state->vao);
    glGenBuffers(1, &state->vbo);
    glGenBuffers(1, &state->instance_vbo);

    glBindVertexArray(state->vao);
    glBindBuffer(GL_ARRAY_BUFFER, state->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, state->instance_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(state->metadata_buffer), NULL, GL_STREAM_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glVertexAttribDivisor(2, 1);

    glGenTextures(1, &state->texture_id);
    glBindTexture(GL_TEXTURE_2D, state->texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 128, 128, 0, GL_RED, GL_UNSIGNED_BYTE, font_data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void render_matrix_frame(GLRenderer* state, ScreenMatrix* matrix) {
    int idx = 0;
    for (int r = 0; r < SCREEN_ROWS; r++) {
        for (int c = 0; c < SCREEN_COLS; c++) {
            GridCell cell = matrix->grid[r * SCREEN_COLS + c];
            state->metadata_buffer[idx++] = (float)cell.character;
            state->metadata_buffer[idx++] = (float)c;
            state->metadata_buffer[idx++] = (float)r;
            state->metadata_buffer[idx++] = (float)(cell.fg_color | (cell.bg_color << 4));
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, state->instance_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(state->metadata_buffer), state->metadata_buffer);

    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(state->program);
    glBindVertexArray(state->vao);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, SCREEN_ROWS * SCREEN_COLS);
}
