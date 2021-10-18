#pragma once
#include <string>
#include <Core/Image.h>

enum class BlendMode : unsigned char {
    NORMAL,
    LIGHTEN
};

class Layer {
public:
    Layer();
    ~Layer();
public:
    Image img_;
    struct {
        std::string name;
        BlendMode   blend_mode;
    } info_;
};