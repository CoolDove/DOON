#version 450 core

// in vec2 os_pos;
// in vec2 ndc_pos;
in vec2 v_uv;
layout (location = 0) out vec4 FragColor;

// uniform vec2 _size;
uniform sampler2D _tex;
// uniform sampler2D _framebuffer;

void main() {
    vec4 src = texture(_tex, vec2(v_uv.x, -v_uv.y));
    FragColor = src;
    // vec2 fbuf_uv = vec2((ndc_pos.x + 1.0f) / 2.0f, (1 - (ndc_pos.y + 1.0f) / 2.0f));
    // vec4 dst = texture(_framebuffer, vec2(fbuf_uv.x, -fbuf_uv.y));
    // float outa = src.a + dst.a * (1 - src.a);
    // FragColor = (src * src.a + dst * dst.a * (1 - src.a)) / outa;
}
