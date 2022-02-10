#version 450 core

in vec2 os_pos;
in vec2 v_uv;

layout (location = 0) out vec4 FragColor;

uniform vec2 _size;
uniform sampler2D _brushtex;
uniform sampler2D _paintbuffer;

void main() {
    vec4 src = texture(_brushtex, vec2(v_uv.x, v_uv.y));
    vec4 dst = texture(_paintbuffer, vec2(v_uv.x, v_uv.y));

    float outa = src.a + dst.a * (1 - src.a);
    vec3 col = (src.rgb * src.a + dst.rgb * dst.a * (1 - src.a)) / outa;

    col.xyz *= dst.a;

    FragColor = vec4(col, outa);
}
