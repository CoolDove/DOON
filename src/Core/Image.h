#pragma once
#include "stb_image/stb_image.h"
#include <Core/Color.h>
#include <Base/General.h>
#include <stdint.h>

class Image {
public:
    Image();
    Image(const char* _path, int _desired_channel);
    Image(unsigned int _width, unsigned int _height, unsigned int _base_color = 0x00000000);
    Image(unsigned int _width, unsigned int _height, Col_RGBA _base_color = Col_RGBA{0x00, 0x00, 0x00, 0x00});
    Image(const Image* _src, Dove::IRect2D _region);
    ~Image();

private:
    // @temp: we will be able to use these functions later
    Image(const Image&) = delete;
    Image& operator=(const Image&) = delete;

public:
    void recreate(unsigned int _width, unsigned int _height, Col_RGBA _base_color = Col_RGBA{0x00, 0x00, 0x00, 0x00});
    void set_subimage(const Image* _img, Dove::IVector2D _pos);

    // TODO: change pixels type to Col_RGBA*
    Col_RGBA* pixels_;
    struct Info {
        int width;
        int height;
        int channels;
    } info_;

private:
    bool inited_;
};
