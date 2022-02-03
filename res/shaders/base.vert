#version 450 core
layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec2 uv;

out vec2 os_pos;

uniform mat4 _view;
uniform mat4 _proj;
uniform vec2 _cansize;

void main() {
    vec3 pos = vec3(_cansize, 0.0) * aPos * 0.5;
    os_pos.x = pos.x;
    os_pos.y = pos.y;

    gl_Position = _proj * (_view * vec4(pos, 1.0f));
}
