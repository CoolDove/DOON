#version 450 core

in vec2 v_uv;
layout (location = 0) out vec4 FragColor;

uniform sampler2D _tex;

void main() {
	// FragColor = vec4(v_uv.x, v_uv.y, 0.0f, 1.0f);
	FragColor = texture(_tex, vec2(v_uv.x, -v_uv.y));
}