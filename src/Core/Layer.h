#pragma once
#include <string>
#include <Core/Image.h>

enum class BlendMode : unsigned char {
    NORMAL,
    LIGHTEN
};

class Layer {
public:
    Layer(unsigned int _width, unsigned int _height);
    ~Layer();
public:
    Image img_;
    struct {
        std::string name;
        BlendMode   blend_mode;
    } info_;
};