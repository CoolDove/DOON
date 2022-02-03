#include "Scene.h"
#include "Base/General.h"
#include "Core/Application.h"
#include "Core/Color.h"
#include "Core/Image.h"
#include "DGLCore/GLEnums.h"
#include "stb_image/stb_image.h"
#include <Core/Sampler.h>
#include <Core/Serialize.h>
#include <DoveLog.hpp>
#include <DGLCore/GLDebugger.h>
#include <atomic>
#include <string.h>
#include <string>
#include <stdint.h>
#include <sys/stat.h>

Scene::Scene(const char* _path)
:   region_{0}
{
    camera_.position_.x = 0.0f;
    camera_.position_.y = 0.0f;
    camera_.size_       = 0.9f;

    if (!load_scene(_path)) {
        create_scene(1024, 1024, Col_RGBA{0xff, 0xff, 0xff, 0x00});
    }

}

Scene::Scene(unsigned int _width, unsigned int _height, Col_RGBA _col)
:   region_{0}
{
    camera_.position_.x = 0.0f;
    camera_.position_.y = 0.0f;
    camera_.size_       = 0.9f;

    create_scene(_width, _height, _col);
}

void Scene::create_scene(uint32_t _width, uint32_t _height, Col_RGBA _col) {
    info_.width  = _width;
    info_.height = _height;

    brush_layer_.init();
    brush_layer_.allocate(1, SizedInternalFormat::RGBA8, _width, _height, PixFormat::RGBA, PixType::UNSIGNED_BYTE);

    BufFlag flag = BufFlag::DYNAMIC_STORAGE_BIT|
                   BufFlag::MAP_READ_BIT|
                   BufFlag::MAP_WRITE_BIT;

    add_layer(_col);

    Dove::IRect2D region;
    region.posx = region.posy = 0;
    region.width = (uint32_t)info_.width;
    region.height = (uint32_t)info_.height;
}

static inline std::string get_file_extension(const std::string& path) {
    int n = path.find_last_of(".");
    if (n == path.length() - 1) return "";

    return path.substr(n + 1, path.length() - n);
}

bool Scene::load_scene(const char* path) {
    auto ext = get_file_extension(path);

    struct stat buffer;
    if (stat(path, &buffer) == -1) return false;

    if      (ext == "png") load_png(path);
    else if (ext == "doo") load_doo(path);
    else return false;

    brush_layer_.init();
    brush_layer_.allocate(1, SizedInternalFormat::RGBA8, info_.width, info_.height, PixFormat::RGBA, PixType::UNSIGNED_BYTE);

    return true;
}

bool Scene::load_png(const char* path) {
    add_layer(path);

    info_.width = get_curr_layer()->info_.width;
    info_.height = get_curr_layer()->info_.height;
    
    Dove::IRect2D region;
    region.posx = region.posy = 0;
    region.width = (uint32_t)info_.width;
    region.height = (uint32_t)info_.height;


    return true;
}

bool Scene::load_doo(const char* path) {
    DooReader reader(path);
    const DooHeader* header = &reader.header;

    uint32_t layers_count = reader.get_layers_count();


    info_.width = header->width;
    info_.height = header->height;

    uint32_t pxcount = info_.width * info_.height;
    uint32_t layer_size = info_.width * info_.height * sizeof(Col_RGBA);

    for (uint32_t i = 0; i < layers_count; i++) {
        add_layer(Col_RGBA{0xff, 0xff, 0xff, 0x00}, reader.get_layer_name(i));
        Layer* layer = get_curr_layer();
        LayerHeader lheader = reader.get_layer_header(i);
        layer->info_.blend_mode = (BlendMode)lheader.blend_mode;

        layer->mem_alloc();
        reader.get_layer_pixels(i, layer->pixels_);
        layer->update_tex();
        layer->mem_release();
    }

    
    return true;
}

void Scene::on_update() {

}

void Scene::add_layer(const std::string& _path) {
    std::string name = _path;   
    layers_.emplace_back(std::make_unique<Layer>(_path));
    curr_layer_ite_ = --layers_.end();
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
