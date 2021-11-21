#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 uv;

out	vec2 v_uv;

uniform mat4 _view;
uniform mat4 _proj;

void main() {
	gl_Position = _proj * (_view * vec4(aPos, 1.0f));
	v_uv = uv;
}
