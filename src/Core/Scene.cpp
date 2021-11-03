#include "Scene.h"
#include "Base/General.h"
#include "stb_image/stb_image.h"
#include <Core/Sampler.h>
#include <DoveLog.hpp>
#include <DGLCore/GLDebugger.h>
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
    brush_img_(_width, _height, _col),
    result_img_(_width, _height, _col),
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

    // TODO: brush tex and img here is not used, remove them someday
    // brush_tex_.init();
    // brush_tex_.allocate(1, SizedInternalFormat::RGBA8, _width, _height);
    // brush_tex_.upload(0, 0, 0, _width, _height, PixFormat::RGBA, PixType::UNSIGNED_BYTE, brush_img_.pixels_);

    // @doing: initialize the result tex
    result_tex_.init();
    result_tex_.allocate(1, SizedInternalFormat::RGBA8, _width, _height);
    result_tex_.upload(0, 0, 0, _width, _height, PixFormat::RGBA, PixType::UNSIGNED_BYTE, result_img_.pixels_);

    // @sec: load compose shader(program)
    DGL::Shader computer;
    computer.init(DGL::ShaderType::COMPUTE_SHADER);

    try {
        computer.load("./res/shaders/brush-composite.comp");
    } catch (const DGL::EXCEPTION::SHADER_COMPILING_FAILED& err) {
        DLOG_ERROR("shader err: %s", err.msg.c_str());
        assert(1 && "failed to compile compute shader");
    }
    compose_shader_.init();
    compose_shader_.link(1, &computer);

    Dove::IRect2D region;
    region.posx = region.posy = 0;
    region.width = (uint32_t)info_.width;
    region.height = (uint32_t)info_.height;
    
    merge_region(region);
}

void Scene::on_update() {
    // #define REGION_UPLOAD
    // FIXME:
    // because the brush doesnt correctly setup the updated region after brush released for now.
    // so we cannot use REGION_UPLOAD, fix this later
    using namespace DGL;

    #ifdef REGION_UPLOAD
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
    #else
    GLTexture2D* tex = &curr_layer_ite_->get()->tex_;
    Image*       img = &curr_layer_ite_->get()->img_;
    tex->upload(0, 0, 0, info_.width, info_.height,
                PixFormat::RGBA, PixType::UNSIGNED_BYTE,
                (Col_RGBA*)img->pixels_);

    #endif
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

void Scene::merge_region(Dove::IRect2D _region) {
    region_ = Dove::merge_rect(_region, region_);
}

void Scene::clear_region() {
    region_ = {0};
}

Scene::~Scene() {
    // ...
}
