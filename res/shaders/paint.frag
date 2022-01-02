#version 450 core

in vec2 os_pos;
in vec2 v_uv;

layout (location = 0) out vec4 FragColor;

uniform vec2 _size;
uniform sampler2D _tex;
uniform sampler2D _paint_buffer;

void main() {
    FragColor = texture(_tex, vec2(v_uv.x, v_uv.y));
}
