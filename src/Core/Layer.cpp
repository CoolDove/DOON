#include "Layer.h"

Layer::Layer() 
:   img_(128, 128, 0x00000000)
{
    info_ = {0};
}
Layer::~Layer() {
}