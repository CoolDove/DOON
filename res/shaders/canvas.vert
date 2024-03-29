﻿#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 uv;

out	vec2 v_uv;
// out vec2 os_pos;
// out vec2 ndc_pos;

uniform mat4 _view;
uniform mat4 _proj;
uniform vec2 _cansize;

void main() {
    vec3 pos = vec3(_cansize, 0.0) * aPos * 0.5;

	gl_Position = _proj * (_view * vec4(pos, 1.0f));
	v_uv = uv;

    // os_pos = vec2(aPos.x, aPos.y);
    // ndc_pos = vec2(gl_Position.x/gl_Position.w, gl_Position.y/gl_Position.w);
}
