#version 450 core

in vec2 os_pos;
in vec2 v_uv;
in vec2 screen_uv;

layout (location = 0) out vec4 FragColor;

uniform vec2 _size;
uniform vec4 _brushcol;
uniform sampler2D _brushtex;
uniform sampler2D _paintbuffer;

void main() {
    vec4 src = texture(_brushtex, v_uv);
    vec4 dst = texture(_paintbuffer, screen_uv);

    src *= _brushcol;
    
    float outa = src.a + dst.a * (1 - src.a);
    vec3 col = (src.rgb * src.a + dst.rgb * dst.a * (1 - src.a)) / outa;

    FragColor = vec4(col, min(_brushcol.a, outa));
}
