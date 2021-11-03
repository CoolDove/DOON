#pragma once
#include "stb_image/stb_image.h"
#include <Core/Color.h>

class Image {
public:
    Image();
    Image(const char* _path, int _desired_channel);
    Image(unsigned int _width, unsigned int _height, unsigned int _base_color = 0x00000000);
    Image(unsigned int _width, unsigned int _height, Col_RGBA _base_color = Col_RGBA{0x00, 0x00, 0x00, 0x00});
    ~Image();

public:
    void recreate(unsigned int _width, unsigned int _height, Col_RGBA _base_color = Col_RGBA{0x00, 0x00, 0x00, 0x00});

    unsigned char* pixels_;
    struct Info {
        int width;
        int height;
        int channels;
    } info_;

private:
    bool inited_;
};
