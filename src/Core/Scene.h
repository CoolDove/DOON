#pragma once

#include "DGLCore/DGLCore.h"
#include "Image.h"
// #include "Layer.h"

#include <list>
#include <memory>

class Scene {
public:
    Scene(const char* _image_path);
    Scene(unsigned int _base_color);
    ~Scene();

    DGL::Camera      camera_;
    Image            image_;
    // std::list<std::unique_ptr<Layer*>> layers_;
};