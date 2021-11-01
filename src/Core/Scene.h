#pragma once

#include <DGLCore/DGLCore.h>
#include <Core/Image.h>
#include <Core/Layer.h>
#include <Core/Color.h>

#include <list>
#include <memory>

struct RectInt {
    // posx and posy is the left-up point
    int posx;
    int posy;
    int width;
    int height;
};

using LayerList = std::list<std::unique_ptr<Layer>>;
using LayerIte = std::list<std::unique_ptr<Layer>>::iterator;

using namespace DGL;

class Scene {
public:
    // Scene(const char* _image_path);
    Scene(uint32_t _width, uint32_t _height, Col_RGBA _col);
    ~Scene();

    void on_render();
    void on_update();

    void add_layer(Col_RGBA _col, const std::string& _name);
    // void add_layer(Col_RGBA _col, const std::string& _name, int _layer_position_or_something);
    void add_layer(Col_RGBA _col);
    void change_layer(const std::string& _name);
    void change_layer(Layer* _layer);
    bool next_layer();
    bool previous_layer();

    void merge_region(RectInt _region);
    void clear_region();

    Layer*  get_curr_layer() { return curr_layer_ite_->get(); };
    RectInt get_region() const { return region_; };
public:
    Camera      camera_;
    // NOTE: maybe this is not necessary anymore
    Image       image_;

    LayerList   layers_;

    GLTexture2D brush_tex_;
    Image       brush_img_;

    struct {
        int width;
        int height;
    } info_;

private:
    RectInt     region_;// updated region
    LayerIte    curr_layer_ite_;
};
