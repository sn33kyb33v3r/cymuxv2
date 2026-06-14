#include "renderer.h"
#include <stdlib.h>
#include <android/log.h>

#define LOG_TAG "CymuxRenderer"

const char* vertex_shader_source = 
    "#version 300 es\n"
    "layout(location = 0) in vec2 a_quad_pos;\n"
    "layout(location = 1) in uint a_char_id;\n"
    "layout(location = 2) in vec2 a_cell_pos;\n"
    "layout(location = 3) in uint a_fg_color;\n"
    "layout(location = 4) in uint a_bg_color;\n"
    "out vec2 v_tex_coord;\n"
    "flat out uint v_fg;\n"
    "flat out uint v_bg;\n"
    "void main() {\n"
    "    gl_Position = vec4(a_quad_pos + a_cell_pos, 0.0, 1.0);\n"
    "    v_tex_coord = a_quad_pos * 0.5 + vec2(0.5);\n"
    "    v_fg = a_fg_color;\n"
    "    v_bg = a_bg_color;\n"
    "}\n";

const char* fragment_shader_source = 
    "#version 300 es\n"
    "precision mediump float;\n"
    "in vec2 v_tex_coord;\n"
    "flat in uint v_fg;\n"
    "flat in uint v_bg;\n"
    "out vec4 fragColor;\n"
    "uniform sampler2D u_texture_atlas;\n"
    "void main() {\n"
    "    vec4 tex = texture(u_texture_atlas, v_tex_coord);\n"
    "    fragColor = vec4(tex.rgb, 1.0);\n"
    "}\n";

static GLuint compile_shader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    return shader;
}

void renderer_init(GlesRenderer* renderer) {
    GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_shader_source);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
    renderer->program = glCreateProgram();
    glAttachShader(renderer->program, vs);
    glAttachShader(renderer->program, fs);
    glLinkProgram(renderer->program);

    float quad_vertices[] = {
        -0.02f,  0.04f,
        -0.02f, -0.04f,
         0.02f, -0.04f,
        -0.02f,  0.04f,
         0.02f, -0.04f,
         0.02f,  0.04f
    };

    glGenVertexArrays(1, &renderer->vao);
    glBindVertexArray(renderer->vao);

    glGenBuffers(1, &renderer->vbo_quad);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_quad);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glGenBuffers(1, &renderer->vbo_metadata);
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_metadata);
    glBufferData(GL_ARRAY_BUFFER, sizeof(renderer->screen_grid), NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(CellMetadata), (void*)offsetof(CellMetadata, char_id));
    glVertexAttribDivisor(1, 1);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(CellMetadata), (void*)offsetof(CellMetadata, x));
    glVertexAttribDivisor(2, 1);

    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(CellMetadata), (void*)offsetof(CellMetadata, fg_color));
    glVertexAttribDivisor(3, 1);

    glEnableVertexAttribArray(4);
    glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT, sizeof(CellMetadata), (void*)offsetof(CellMetadata, bg_color));
    glVertexAttribDivisor(4, 1);

    glGenTextures(1, &renderer->texture_atlas);
    glBindTexture(GL_TEXTURE_2D, renderer->texture_atlas);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void renderer_update_grid(GlesRenderer* renderer, const uint8_t* screen_matrix) {
    for (int r = 0; r < GRID_ROWS; ++r) {
        for (int c = 0; c < GRID_COLS; ++c) {
            int idx = r * GRID_COLS + c;
            renderer->screen_grid[idx].char_id = screen_matrix[idx];
            renderer->screen_grid[idx].x = -0.95f + (c * 0.024f);
            renderer->screen_grid[idx].y =  0.90f - (r * 0.050f);
            renderer->screen_grid[idx].fg_color = 0xFFFFFFFF;
            renderer->screen_grid[idx].bg_color = 0x00000000;
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, renderer->vbo_metadata);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(renderer->screen_grid), renderer->screen_grid);
}

void renderer_render_frame(GlesRenderer* renderer) {
    glUseProgram(renderer->program);
    glBindVertexArray(renderer->vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderer->texture_atlas);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, GRID_ROWS * GRID_COLS);
}
