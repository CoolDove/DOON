#include "Image.h"
#include "Base/General.h"
#include "Core/Color.h"
#include "assert.h"
#include "Windows.h"

Image::Image()
:   inited_(false),
    info_{0}      
{
}

// TODO: append alpha channel for 3-channel-images
Image::Image(const char *_path, int _desired_channel)
:   inited_(false)
{
    pixels_ = (Col_RGBA*)stbi_load(_path, &info_.width, &info_.height, &info_.channels, _desired_channel);

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

// TODO: optimize this
Image::Image(const Image *_src, Dove::IRect2D _region)
:   inited_(false),
    info_{0}
{
    // @robust:
    if (!_src) return;

    auto clamp = [](int _v, int _down, int _up) -> int {
        assert(_down < _up);
        if (_v < _down) return _down;
        if (_v > _up)   return _up;
        return _v;
    };
    _region.posx   = clamp(_region.posx, 0, _src->info_.width);
    _region.posy   = clamp(_region.posy, 0, _src->info_.height);
    _region.width  = clamp(_region.width, 0, _src->info_.width - _region.posx);
    _region.height = clamp(_region.height, 0, _src->info_.height - _region.posy);
    if (_region.width == 0 || _region.height == 0) return;

    // @load:
    info_.width = _region.width;
    info_.height = _region.height;
    pixels_ = (Col_RGBA*)malloc(info_.width * info_.height * sizeof(Col_RGBA));
    Col_RGBA* ptr_dst = pixels_;
    Col_RGBA* ptr_src = _src->pixels_ + _region.posx;
    for (int i = _region.posy; i < _region.posy + _region.height; i++) {
        memcpy(ptr_dst, ptr_src, _region.width * sizeof(Col_RGBA));
        ptr_dst += _region.width;
        ptr_src += _src->info_.width;
    }
}

void Image::recreate(unsigned int _width, unsigned int _height, Col_RGBA _base_color) {
    if (inited_) free(pixels_);

    info_.width    = _width;
    info_.height   = _height;
    info_.channels = 4;

    pixels_ = (Col_RGBA*)malloc(_width * _height * 4);
    Col_RGBA* pix = (Col_RGBA*)pixels_;

    if (_base_color.cluster == 0x00000000) {
        memset(pix, 0, _width * _height * 4);
    } else {
        for (uint32_t i = 0; i < _width * _height; i++) 
            pix[i] = _base_color;
    }

    inited_ = true;
}

void Image::set_subimage(const Image *_img, Dove::IVector2D _pos) {
    Dove::IRect2D region_src; // for _img
    Dove::IRect2D region_dst; // for this

    // @prepare: set the region_src and region_dst
    auto clamp = [](int _v, int _down, int _up) -> int {
        assert(_down < _up);
        if (_v < _down) return _down;
        if (_v > _up)   return _up;
        return _v;
    };

    if (_pos.x > info_.width || _pos.y > info_.height) return;
    if (_pos.x < -_img->info_.width || _pos.y < -_img->info_.height) return;

    if (_pos.x >= 0) {
        region_src.posx = 0;
        region_src.width = min(_img->info_.width, info_.width - _pos.x);

        region_dst.posx = _pos.x;
        region_dst.width = region_src.width;
    } else {
        region_src.posx = -_pos.x;
        region_src.width = min(_img->info_.width - region_src.posx, info_.width);

        region_dst.posx = 0;
        region_dst.width = region_src.width;
    }

    if (_pos.y >= 0) {
        region_src.posy = 0;
        region_src.height = min(_img->info_.height, info_.height - _pos.y);

        region_dst.posy = _pos.y;
        region_dst.height = region_src.height;
    } else {
        region_src.posy = -_pos.y;
        region_src.height = min(_img->info_.height - region_src.posy, info_.height);

        region_dst.posy = 0;
        region_dst.height = region_src.height;
    }
    // @set:
    Col_RGBA* ptr_src = _img->pixels_ + region_src.posy * _img->info_.width + region_src.posx;
    Col_RGBA* ptr_dst = pixels_ + region_dst.posy * info_.width + region_dst.posx;

    for (int i = 0; i < region_src.height; i++) {
        memcpy(ptr_dst, ptr_src, region_src.width * sizeof(Col_RGBA));
        ptr_src += _img->info_.width;
        ptr_dst += info_.width;
    }
}

Image::~Image() {
    if (inited_) free(pixels_);
}
