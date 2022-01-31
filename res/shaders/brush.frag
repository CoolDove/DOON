#version 450 core

in vec2 v_uv;

layout (location = 0) out vec4 FragColor;

uniform sampler2D _brushtex;
uniform vec4 _brushcol;

void main() {
    vec4 col = texture(_brushtex, v_uv);
    col *= _brushcol;
    FragColor = col;
}
