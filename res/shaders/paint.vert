#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 uv;

out	vec2 v_uv;
out vec2 os_pos;

uniform vec2 _size;

void main() {
	v_uv = uv;
    os_pos = vec2(aPos.x, aPos.y);
    vec2 ndc = _size;
    ndc.x = (2 * os_pos.x - _size.x) / _size.x + 1;
    ndc.y = (2 * os_pos.y - _size.y) / _size.y + 1;
    gl_Position = vec4(ndc, 0.0, 1.0);

}
