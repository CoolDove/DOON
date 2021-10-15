#pragma once

#include "DGLCore/DGLCore.h"
#include "Image.h"

class Scene {
public:
    Scene(const char* _image_path);
    Scene(unsigned int _base_color);
    ~Scene();

    DGL::Camera     camera_;
    DGL::GeoBatch   batch_;
    Image           image_;
};