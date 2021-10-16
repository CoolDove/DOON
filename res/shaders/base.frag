#version 450 core

in vec2 os_pos;
layout (location = 0) out vec4 FragColor;
uniform vec2 _size;
uniform int _scale;

void main() {
    // int scale = clamp(_scale, 5, 10);
    int cell = 1 * _scale;
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

    vec4 col_a = vec4(0.7f, 0.7f, 0.7f, 1.0f);
    vec4 col_b = vec4(0.8f, 0.8f, 0.8f, 1.0f);
    // vec4 col_a = vec4(0.1f, 0.8f, 0.6f, 1.0f);
    // vec4 col_b = vec4(0.7f, 0.8f, 0.2f, 1.0f);

    FragColor = mix(col_a, col_b, mix_);

}