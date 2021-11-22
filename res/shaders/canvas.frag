#version 450 core
// #extension GL_EXT_shader_framebuffer_fetch : require
#extension GL_EXT_shader_framebuffer_fetch: enable

in vec2 v_uv;
layout (location = 0) out vec4 FragColor;

uniform sampler2D _tex;

void main() {
	FragColor = texture(_tex, vec2(v_uv.x, -v_uv.y));
    // FragColor = FragColor * gl_LastFragData[0];
    // FragColor.a = 1.0;
}
