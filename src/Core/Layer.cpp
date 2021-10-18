#include "Layer.h"

Layer::Layer(unsigned int _width, unsigned int _height, std::string _name, Col_RGBA _col) 
:   img_(_width, _height, _col)
{
    info_.name       = _name;
    info_.blend_mode = BlendMode::NORMAL;
}

Layer::~Layer() {

}