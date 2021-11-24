#pragma once

#include "DGLCore/GLTexture.h"
#include <DGLCore/DGLCore.h>
#include <Core/Image.h>
#include <Core/Layer.h>
#include <Core/Color.h>
#include <Base/General.h>

#include <list>
#include <memory>

using LayerList = std::list<std::unique_ptr<Layer>>;
using LayerIte = std::list<std::unique_ptr<Layer>>::iterator;
using LayerPtr = std::unique_ptr<Layer>;

using namespace DGL;

class Scene {
public:
    Scene(const char* _image_path);
    Scene(uint32_t _width, uint32_t _height, Col_RGBA _col);
    ~Scene();

    void on_render();
    void on_update();

    void add_layer(Col_RGBA _col, const std::string& _name);
    void add_layer(Col_RGBA _col);
    void change_layer(const std::string& _name);
    void change_layer(Layer* _layer);
    bool next_layer();
    bool previous_layer();

    void mark_region(Dove::IRect2D _region);
    void clear_region();

    Layer*        get_curr_layer() { return curr_layer_ite_->get(); };
    Dove::IRect2D get_region() const { return region_; };
public:
    Camera      camera_;

    LayerList   layers_;
    LayerPtr    brush_layer_;

    struct {
        int width;
        int height;
    } info_;

private:
    Dove::IRect2D region_; // updated region
    LayerIte      curr_layer_ite_;
};
