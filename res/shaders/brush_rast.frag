#version 450 core

in vec2 os_pos;
in vec2 v_uv;
in vec2 screen_uv;
in float radius;

layout (location = 0) out vec4 FragColor;

uniform vec2 _size;
uniform vec4 _brushcol;
uniform sampler2D _brushtex;
uniform sampler2D _paintbuffer;
uniform vec2 _canvassize;

void main() {
    vec4 src;
    if (radius < 2.0) {
        src = vec4(1.0);
    } else {
        src = texture(_brushtex, v_uv);
    }

    vec4 dst = texture(_paintbuffer, screen_uv);

    src *= _brushcol;
    
    float outa = src.a + dst.a * (1 - src.a);
    vec3 col = (src.rgb * src.a + dst.rgb * dst.a * (1 - src.a)) / outa;

    float alpha = min(_brushcol.a, outa);

    vec2 rast = screen_uv * 100.0;
    rast.x = step(rast.x - int(rast.x), 0.5);
    rast.y = step(rast.y - int(rast.y), 0.5);

    float r = 0.0;
    if (rast.x == rast.y) r = 1.0;
    alpha *= r;
    // col = vec3(r, r, r);

    FragColor = vec4(col, alpha);
}
