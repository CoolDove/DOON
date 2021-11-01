#include "Image.h"
#include "assert.h"
#include "Windows.h"

// TODO: append alpha channel for 3-channel-images
Image::Image()
:   inited_(false),
    info_{0}      
{
}    

Image::Image(const char *_path, int _desired_channel)
:   inited_(false)
{
    pixels_ = stbi_load(_path, &info_.width, &info_.height, &info_.channels, _desired_channel);

    // @temp: to be replaced by channel appending
    assert(info_.channels > 3); // 3-channel-image, cannot be used for now, need to be append
    assert(pixels_);            // falied to load

    inited_ = true;
}

Image::Image(unsigned int _width, unsigned int _height, Col_RGBA _base_color)
:   inited_(false)
{
    recreate(_width, _height, _base_color);
}

void Image::recreate(unsigned int _width, unsigned int _height, Col_RGBA _base_color) {
    if (inited_) free(pixels_);

    info_.width    = _width;
    info_.height   = _height;
    info_.channels = 4;

    pixels_ = (unsigned char*)malloc(_width * _height * 4);
    Col_RGBA* pix = (Col_RGBA*)pixels_;

    if (_base_color.cluster == 0x00000000) {
        memset(pix, 0, _width * _height * 4);
    } else {
        for (uint32_t i = 0; i < _width * _height; i++) 
            pix[i] = _base_color;
    }

    inited_ = true;
}

Image::~Image() {
    if (inited_) free(pixels_);
}
