#include "Scene.h"
#include "stb_image/stb_image.h"
#include <Core/Sampler.h>
#include <DoveLog.hpp>
#include <DGLCore/GLDebugger.h>

// Scene::Scene(const char* _image_path)
// :   image_(_image_path, 0)
// {
//     camera_.position_.x = 0.0f;
//     camera_.position_.y = 0.0f;
//     camera_.size_       = 5.0f;

//     info_.width  = image_.info_.width;
//     info_.height = image_.info_.height;
// }

Scene::Scene(unsigned int _width, unsigned int _height, Col_RGBA _col)
:   image_(_width, _height, _col),
    region_{0},
    brush_img_(_width, _height, _col)
{
    camera_.position_.x = 0.0f;
    camera_.position_.y = 0.0f;
    camera_.size_       = 5.0f;

    info_.width  = image_.info_.width;
    info_.height = image_.info_.height;

    BufFlag flag = BufFlag::DYNAMIC_STORAGE_BIT|
                   BufFlag::MAP_READ_BIT|
                   BufFlag::MAP_WRITE_BIT;


    add_layer(_col);

    brush_tex_.init();
    brush_tex_.allocate(1, SizedInternalFormat::RGBA8, _width, _height);
    brush_tex_.upload(0, 0, 0, _width, _height, PixFormat::RGBA, PixType::UNSIGNED_BYTE, brush_img_.pixels_);

    merge_region({0, 0, info_.width, info_.height});
}

void Scene::on_update() {
    // TODO: upload the updated region of current layer
    using namespace DGL;
    if (region_.width != 0 && region_.height != 0) {
        GLTexture2D* tex = &curr_layer_ite_->get()->tex_;
        Image*       img = &curr_layer_ite_->get()->img_;

        for (int i = region_.posy; i < region_.posy + region_.height; i++) {
            tex->upload(0, region_.posx, i, region_.width, 1,
                        PixFormat::RGBA, PixType::UNSIGNED_BYTE,
                        (Col_RGBA*)img->pixels_ + i * info_.width + region_.posx);
        }

        clear_region();
    }
}

void Scene::add_layer(Col_RGBA _col) {
    if (layers_.size() == 0) 
        add_layer(_col, "base");
    else 
        add_layer(_col, "layer" + std::to_string(layers_.size() - 1));
}

void Scene::add_layer(Col_RGBA _col, const std::string& _name) {
    layers_.emplace_back(std::make_unique<Layer>(
        info_.width,
        info_.height,
        _name,
        _col
    ));
    
    curr_layer_ite_ = --layers_.end();
}

void Scene::change_layer(const std::string& _name) {
    // to be done
}

void Scene::change_layer(Layer* _layer) {
    for (auto ite = layers_.begin(); ite != layers_.end(); ite++)
    {
        if (ite->get() == _layer) {
            curr_layer_ite_ = ite;
        }
    }
    assert("you have to input a Layer* that is in the layers_ list");
}

bool Scene::next_layer() {
    LayerIte ite = curr_layer_ite_;
    if (++ite != layers_.end()) {
        curr_layer_ite_ = ite;
        return true;
    } else {
        return false;
    }
}
bool Scene::previous_layer() {
    if (curr_layer_ite_ != layers_.begin()) {
        curr_layer_ite_--;
        return true;
    }
    return false;
}

void Scene::merge_region(RectInt _region) {
    if (_region.width == 0 || _region.height == 0) {
        int x = glm::min(_region.posx, region_.posx);
        int y = glm::min(_region.posy, region_.posy);
        int w = glm::max(_region.width + _region.posx, region_.width + region_.posx) - x;
        int h = glm::max(_region.height + _region.posy, region_.height + region_.posy) - y;
        region_ = {x, y, w, h};
    }
    else {
        region_ = _region;
    }
}

void Scene::clear_region() {
    region_ = {0};
}

Scene::~Scene() {
    // ...
}
