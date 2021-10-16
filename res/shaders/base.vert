#version 450 core
layout (location = 0) in vec3 aPos;
// layout (location = 1) in vec2 uv;

out vec2 os_pos;

uniform mat4 _view;
uniform mat4 _proj;

void main() {
    os_pos.x = aPos.x;
    os_pos.y = aPos.y;

    gl_Position = _proj * (_view * vec4(aPos, 1.0f));
}