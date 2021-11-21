﻿#include "Scene.h"
#include "Base/General.h"
#include "Core/Application.h"
#include "Core/Color.h"
#include "Core/Compositor.h"
#include "Core/Image.h"
#include "DGLCore/GLEnums.h"
#include "stb_image/stb_image.h"
#include <Core/Sampler.h>
#include <DoveLog.hpp>
#include <DGLCore/GLDebugger.h>
#include <atomic>
#include <cstring>
#include <stdint.h>

// TODO: change texture filering mode after initializing, for layer textures and the brush texture.

Scene::Scene(const char* _image_path)
:   region_{0}
{
    camera_.position_.x = 0.0f;
    camera_.position_.y = 0.0f;
    camera_.size_       = 5.0f;

    Image img(_image_path, 0);

    info_.width  = img.info_.width;
    info_.height = img.info_.height;
    
    // you must set info_ before adding layers
    add_layer(Col_RGBA{0x00, 0x00, 0x00, 0x00}, _image_path);// current layer has been setted
    memcpy(get_curr_layer()->img_->pixels_, img.pixels_, img.get_size_b());
    get_curr_layer()->update_tex(true);
    
    brush_img_ = std::make_unique<Image>(
        info_.width, info_.height, Col_RGBA{0x00, 0x00, 0x00, 0x00});

    Dove::IRect2D region;
    region.posx = region.posy = 0;
    region.width = (uint32_t)info_.width;
    region.height = (uint32_t)info_.height;

    brush_tex_.init();
    brush_tex_.param_mag_filter(TexFilter::NEAREST);
    brush_tex_.param_min_filter(TexFilter::NEAREST);
    brush_tex_.param_wrap_r(TexWrap::CLAMP_TO_EDGE);
    brush_tex_.param_wrap_s(TexWrap::CLAMP_TO_EDGE);

    brush_tex_.allocate(1, SizedInternalFormat::RGBA8, info_.width, info_.height);

    mark_region(region);
}

Scene::Scene(unsigned int _width, unsigned int _height, Col_RGBA _col)
:   region_{0}
{
    camera_.position_.x = 0.0f;
    camera_.position_.y = 0.0f;
    camera_.size_       = 5.0f;

    info_.width  = _width;
    info_.height = _height;

    brush_img_ = std::make_unique<Image>(
        info_.width, info_.height, Col_RGBA{0x00, 0x00, 0x00, 0x00});

    BufFlag flag = BufFlag::DYNAMIC_STORAGE_BIT|
                   BufFlag::MAP_READ_BIT|
                   BufFlag::MAP_WRITE_BIT;

    add_layer(_col);
    get_curr_layer()->update_tex(true);

    Dove::IRect2D region;
    region.posx = region.posy = 0;
    region.width = (uint32_t)info_.width;
    region.height = (uint32_t)info_.height;

    brush_tex_.init();
    brush_tex_.allocate(1, SizedInternalFormat::RGBA8, _width, _height);
    
    mark_region(region);
}


void Scene::on_update() {
    using namespace DGL;
    // update brush layer
    // TODO: region uploading here
    // NOTE: maybe i should have partially updating in texture class.
    brush_tex_.upload(0, 0, 0,
                      info_.width, info_.height,
                      PixFormat::RGBA, PixType::UNSIGNED_BYTE,
                      brush_img_->pixels_);
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

void Scene::mark_region(Dove::IRect2D _region) {
    region_ = Dove::merge_rect(_region, region_);
}

void Scene::clear_region() {
    region_ = {0};
}

Scene::~Scene() {
    // ...
}
