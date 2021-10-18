#include "Layer.h"

Layer::Layer(unsigned int _width, unsigned int _height) 
:   img_(_width, _height, 0x00000000)
{
    info_ = {0};
}
Layer::~Layer() {
}