#pragma once
#include <Core/Image.h>
#include <Core/Color.h>
#include <assert.h>

class Sampler {
public:
    static Col_RGBA sample(Image* _img, int _x, int _y) {

    };

    // static uint32_t pos_to_byte_index(uint32_t _x, uint32_t _y, uint32_t _width, uint32_t _height) {
    //     assert(_x < _width  && "tex pos goes beyond range");
    //     assert(_y < _height && "tex pos goes beyond range");

    //     return _y * _width * 4 + _x * 4;
    // }
};