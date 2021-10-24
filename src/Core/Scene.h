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


    void update(RectInt _region);
    void comfirm_update();

    void add_layer(Col_RGBA _col, const std::string& _name);
    void add_layer(Col_RGBA _col);
    // TODO:...
    void change_layer(const std::string& _name);
    void change_layer(Layer* _layer);
    bool next_layer();
    bool previous_layer();
    const Layer* get_curr_layer() const { return curr_layer_ite_->get(); };

public:
    DGL::Camera camera_;
    Image       image_;
    LayerList   layers_;

    RectInt     region_;// updated region

    struct {
        int width;
        int height;
    } info_;

private:
    // Layer*      curr_layer_;
    LayerIte    curr_layer_ite_;
    
    
private:
    // shouldnt belong here
    DGL::Program composition_cshader_;
    DGL::GLTextureBuffer cache_up_;
    DGL::GLTextureBuffer cache_down_;

    void composite_region(RectInt _region);
    void composite_cache();

};