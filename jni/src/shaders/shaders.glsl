#ifdef VERTEX_SHADER
layout(location = 0) in vec2 in_position;
layout(location = 1) in vec2 in_texcoord;
layout(location = 2) in vec4 in_cell_meta; // x: char_id, y: x_pos, z: y_pos, w: packed_colors

out vec2 frag_texcoord;
out vec3 frag_colors; // r: fg, g: bg, b: flags

void main() {
    float cell_x = in_cell_meta.y;
    float cell_y = in_cell_meta.z;
    
    vec2 pos = in_position + vec2(cell_x * 2.0 / 160.0 - 1.0, 1.0 - cell_y * 2.0 / 45.0);
    gl_Position = vec4(pos, 0.0, 1.0);
    
    float char_id = in_cell_meta.x;
    float atlas_col = mod(char_id, 16.0);
    float atlas_row = floor(char_id / 16.0);
    
    frag_texcoord = (in_texcoord + vec2(atlas_col, atlas_row)) / 16.0;
    
    float packed_c = in_cell_meta.w;
    frag_colors.r = mod(packed_c, 16.0) / 15.0;
    frag_colors.g = floor(packed_c / 16.0) / 15.0;
}
#endif

#ifdef FRAGMENT_SHADER
precision mediump float;
in vec2 frag_texcoord;
in vec3 frag_colors;
uniform sampler2D font_atlas;
out vec4 out_color;

void main() {
    vec4 tex = texture(font_atlas, frag_texcoord);
    vec3 final_rgb = mix(vec3(frag_colors.g), vec3(frag_colors.r), tex.r);
    out_color = vec4(final_rgb, 1.0);
}
#endif
