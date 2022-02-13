#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 uv;

out	vec2 v_uv;
out vec2 screen_uv;

uniform vec2 _dappos;
uniform vec2 _canvassize;
uniform float _dapsize;

void main() {
    v_uv = uv;
    vec2 hcansize = 0.5 * _canvassize;
    vec2 wp = aPos.xy * _dapsize + _dappos;
    wp = (wp - hcansize) / hcansize;
    gl_Position = vec4(wp, 1, 1);
    screen_uv = (wp + 1.0) * 0.5;
}

