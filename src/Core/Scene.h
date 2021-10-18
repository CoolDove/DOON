#pragma once

#include "DGLCore/DGLCore.h"
#include "Image.h"
#include "Layer.h"

#include <list>
#include <memory>

#define uptr(type) std::unique_ptr<type>
using namespace std;

struct RectInt {
    int posx;
    int posy;
    int width;
    int height;
};

class Scene {
public:
    Scene(const char* _image_path);
    Scene(unsigned int _base_color);
    ~Scene();

    void update(RectInt _region);
    void comfirm_update();

    DGL::Camera         camera_;
    Image               image_;
    list<uptr(Layer)>   layers_;

    RectInt             region_;
};