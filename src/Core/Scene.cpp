#include "Scene.h"
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
    result_(_width, _height, _col, true),
    brush_img_(_width, _height, _col),
    region_{0}
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

    Dove::IRect2D region;
    region.posx = region.posy = 0;
    region.width = (uint32_t)info_.width;
    region.height = (uint32_t)info_.height;

    result_buffer_.init();
    result_buffer_.allocate(_width * _height * sizeof(Col_RGBA), flag, SizedInternalFormat::RGBA8UI);
    brush_buffer_.init();
    brush_buffer_.allocate(_width * _height * sizeof(Col_RGBA), flag, SizedInternalFormat::RGBA8UI);
    
    mark_region(region);
}

// TODO: optimization here, use down cache
void Scene::on_update() {
    using namespace DGL;
    if (region_.width != 0 && region_.height != 0) {
        // NOTE: recompose every pixel for every layer, it's horribly slow, but just temp solution for testing
        // @doing: 
        Compositor* compositor = Application::instance_->compositor_.get();
        uint32_t region_size_b = region_.width * region_.height * sizeof(Col_RGBA);
        uint32_t img_size_b    = info_.width * info_.height * sizeof(Col_RGBA);

        // update brush buffer, upload the brush_img_ data to brush_buffer_
        Col_RGBA* brush_buffer_ptr = (Col_RGBA*)brush_buffer_.buffer_->map(Access::READ_WRITE);
        memcpy(brush_buffer_ptr, brush_img_.pixels_, img_size_b);
        brush_buffer_.buffer_->unmap();

        // clear result buffer to start composition
        Col_RGBA* ptr = (Col_RGBA*)result_buffer_.buffer_->map(Access::READ_WRITE);
        memset(ptr, 0x00, img_size_b);
        result_buffer_.buffer_->unmap();
        
        for (auto ite = layers_.begin(); ite != layers_.end(); ite++) {
            compositor->compose("common", &ite->get()->texbuf_, &result_buffer_, img_size_b);
            // @ComposeBrushLayerIn:

            if (ite == curr_layer_ite_) {
                compositor->compose("common", &brush_buffer_, &result_buffer_, img_size_b);
            }
        }

        // we have compose all the layers into result_buffer_, pick it out
        ptr = (Col_RGBA*)result_buffer_.buffer_->map(Access::READ_WRITE);
        memcpy(result_.img_->pixels_, ptr, img_size_b);
        result_buffer_.buffer_->unmap();
        result_.update_tex(true);

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

void Scene::mark_region(Dove::IRect2D _region) {
    region_ = Dove::merge_rect(_region, region_);
}

void Scene::clear_region() {
    region_ = {0};
}

Scene::~Scene() {
    // ...
}
