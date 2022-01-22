#version 450 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 uv;

out vec2 v_uv;

uniform vec2 _dappos;
uniform float _dapsize;

void main() {
    v_uv = uv;
    vec2 p = aPos * _dapsize;
    gl_Position = vec4(p, 0, 1);
}


