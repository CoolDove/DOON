#pragma once
#include <string>
#include <Core/Image.h>
#include <DGLCore/GLTexture.h>
#include "Core/Color.h"

enum class BlendMode : unsigned char {
    NORMAL,
    LIGHTEN
};

class Layer {
public:
    Layer(unsigned int _width, unsigned int _height, std::string _name, Col_RGBA _col);
    ~Layer();
public:
    Image img_;
    struct {
        std::string name;
        BlendMode   blend_mode;
    } info_;

    DGL::GLTexture2D tex_;

};