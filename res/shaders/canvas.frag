#version 450 core

in vec2 v_uv;
layout (location = 0) out vec4 FragColor;

uniform sampler2D _tex;

void main() {
	FragColor = texture(_tex, vec2(v_uv.x, -v_uv.y));
}