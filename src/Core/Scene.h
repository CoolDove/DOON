#pragma once

#include <DGLCore/DGLCore.h>
#include <Core/Image.h>
#include <Core/Layer.h>
#include <Core/Color.h>

#include <list>
#include <memory>

struct RectInt {
    int posx;
    int posy;
    int width;
    int height;
};

using LayerList = std::list<std::unique_ptr<Layer>>;
using LayerIte = std::list<std::unique_ptr<Layer>>::iterator;
class Scene {
public:
    // Scene(const char* _image_path);
    Scene(uint32_t _width, uint32_t _height, Col_RGBA _col);
    ~Scene();

    void add_layer(Col_RGBA _col);

    void update(RectInt _region);
    void comfirm_update();

public:
    DGL::Camera camera_;
    Image       image_;
    LayerList   layers_;
    Layer*      curr_layer_;

    RectInt     region_;// updated region

    struct {
        int width;
        int height;
    } info_;
};