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
    Scene(unsigned int _width, unsigned int _height, unsigned int _base_color);
    ~Scene();

    void add_layer();

    void update(RectInt _region);
    void comfirm_update();

    DGL::Camera         camera_;
    Image               image_;
    list<uptr(Layer)>   layers_;

    RectInt             region_;

    struct {
        int width;
        int height;
    } info_;
};