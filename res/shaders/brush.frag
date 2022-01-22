#version 450 core

in vec2 v_uv;

layout (location = 0) out vec4 FragColor;

uniform sampler2D _brushtex;

void main() {
    FragColor = texture(_brushtex, v_uv);
}
