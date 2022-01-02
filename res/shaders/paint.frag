#version 450 core

in vec2 os_pos;
in vec2 v_uv;

layout (location = 0) out vec4 FragColor;

uniform vec2 _size;
uniform sampler2D _tex;
uniform sampler2D _paintbuffer;

vec4 u2f(uvec4 _ucol) {
    vec4 col;
    col.r = float(_ucol.r) / float(0xff);
    col.g = float(_ucol.g) / float(0xff);
    col.b = float(_ucol.b) / float(0xff);
    col.a = float(_ucol.a) / float(0xff);
    return col;
}

uvec4 f2u(vec4 _fcol) {
    uvec4 col;
    col.r = uint(_fcol.r * 0xff);
    col.g = uint(_fcol.g * 0xff);
    col.b = uint(_fcol.b * 0xff);
    col.a = uint(_fcol.a * 0xff);
    return col;
}

void main() {
    vec4 src = texture(_tex, vec2(v_uv.x, v_uv.y));
    vec4 dst = texture(_paintbuffer, vec2(v_uv.x, v_uv.y));

    float outa = src.a + dst.a * (1 - src.a);
    vec3 col = (src.rgb * src.a + dst.rgb * (1 - src.a)) / outa;

    FragColor = vec4(col, outa);
}
