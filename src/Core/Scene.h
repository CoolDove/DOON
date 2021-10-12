#pragma once

#include "DGLCore/DGLCore.h"
#include "Image.h"

class Scene {
public:
    Scene(const char* _image_path);
    ~Scene();

    DGL::Camera     camera_;
    // DGL::Shader     shader_;// should be in Renderer, now placed in Application
    DGL::GeoBatch   batch_;
    Image           image_;
};