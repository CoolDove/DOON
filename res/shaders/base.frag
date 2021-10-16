#version 450 core

in vec2 os_pos;
in vec2 v_uv;
layout (location = 0) out vec4 FragColor;
uniform vec2 _size;

void main() {
    // FragColor = vec4(v_uv.x, v_uv.y, 0.0f, 1.0f);
    int cell = 5;
    int dcell = 2 * cell;

    int osx = int(os_pos.x + _size.x * 0.5f);
    int osy = int(os_pos.y + _size.y * 0.5f);

    osx = osx % dcell;
    osy = osy % dcell;

    float ff_x = float(osx);
    float ff_y = float(osy);

    ff_x = ff_x/dcell;
    ff_y = ff_y/dcell;

    int x_flag = int(ff_x + 0.5f);
    int y_flag = int(ff_y + 0.5f);

    int mix_ = abs(x_flag - y_flag);

    vec4 col_a = vec4(0.6f, 0.6f, 0.6f, 1.0f);
    vec4 col_b = vec4(0.8f, 0.8f, 0.8f, 1.0f);

    FragColor = mix(col_a, col_b, mix_);

}