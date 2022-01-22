#version 450 core

in vec2 v_uv;

layout (location = 0) out vec4 FragColor;

uniform sampler2D _brushtex;

void main() {
    vec4 col = texture(_brushtex, v_uv);
    FragColor = vec4(0.0, 0.0, 0.0, col.a * 0.4);
}
