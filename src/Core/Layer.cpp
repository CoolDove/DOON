#include "Layer.h"

Layer::Layer(unsigned int _width, unsigned int _height, std::string _name, Col_RGBA _col) 
:   img_(_width, _height, _col)
{
    info_.name       = _name;
    info_.blend_mode = BlendMode::NORMAL;

    tex_.init();
    tex_.allocate(1, DGL::SizedInternalFormat::RGBA8, _width, _height);
    tex_.upload(0, 0, 0, _width, _height, DGL::PixFormat::RGBA, DGL::PixType::UNSIGNED_BYTE, img_.pixels_);
    tex_.param_mag_filter(DGL::TexFilter::NEAREST);
    tex_.param_min_filter(DGL::TexFilter::NEAREST);
    tex_.param_wrap_r(DGL::TexWrap::CLAMP_TO_EDGE);
    tex_.param_wrap_s(DGL::TexWrap::CLAMP_TO_EDGE);
}

Layer::~Layer() {

}