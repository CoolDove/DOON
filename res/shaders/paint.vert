#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 uv;

out	vec2 v_uv;
out vec2 os_pos;

uniform vec2 _size;

void main() {
	v_uv = uv;
    os_pos = vec2(aPos.x, aPos.y);
    vec2 ndc;

    ndc = os_pos / (_size * 0.5);

    gl_Position = vec4(ndc, 0.0, 1.0);
}
