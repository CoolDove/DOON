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
:   image_(_width, _height, _col)
{
    camera_.position_.x = 0.0f;
    camera_.position_.y = 0.0f;
    camera_.size_       = 5.0f;

    info_.width  = image_.info_.width;
    info_.height = image_.info_.height;

    DGL::BufFlag flag = DGL::BufFlag::DYNAMIC_STORAGE_BIT|
                        DGL::BufFlag::MAP_READ_BIT|
                        DGL::BufFlag::MAP_WRITE_BIT;
    cache_up_.init();
    cache_up_.allocate(info_.width * info_.height * sizeof(Col_RGBA), flag, DGL::SizedInternalFormat::RGBA8UI);
    cache_down_.init();
    cache_down_.allocate(info_.width * info_.height * sizeof(Col_RGBA), flag, DGL::SizedInternalFormat::RGBA8UI);

    try
    {
        DGL::Shader comp_shader("./res/shaders/Test.comp", DGL::ShaderType::COMPUTE_SHADER);
        composition_cshader_.init();
        composition_cshader_.link(1, &comp_shader);
    }
    catch(const DGL::EXCEPTION::SHADER_COMPILING_FAILED& err)
    {
        DLOG_ERROR("compute shader compiling failed: %s", err.msg.c_str());
        assert(1);
    }

    add_layer(_col);
    update({0, 0, info_.width, info_.height});

}

void Scene::composite_cache() {
    // composite the unselected layers to cache after switching layer or changing the order
    using namespace DGL;

    size_t  byte_size = info_.width * info_.height * sizeof(Col_RGBA);
    BufFlag bflag_readwrite = BufFlag::DYNAMIC_STORAGE_BIT|
                              BufFlag::MAP_READ_BIT|
                              BufFlag::MAP_WRITE_BIT;
    SizedInternalFormat internal_format = SizedInternalFormat::RGBA8UI; 

    auto ite = layers_.begin();
    uint32_t count = 0;
    // composite cache_down
    while(1) {
        if (ite == curr_layer_ite_) {
            if (count == 0) {
                void* ptr = cache_down_.buffer_->map(Access::READ_WRITE);
                memset(ptr, 0x00, byte_size);
                cache_down_.buffer_->unmap();
            }
            break;
        } else {
            if (count == 0) {
                void* ptr = cache_down_.buffer_->map(Access::READ_WRITE);
                memcpy(ptr, ite->get()->img_.pixels_, byte_size);
                cache_down_.buffer_->unmap();
            } else {
                // composition
                GLTextureBuffer* dst = &cache_down_;
                GLTextureBuffer src;
                src.init();
                src.allocate(byte_size, bflag_readwrite, internal_format);

                void* src_data = src.buffer_->map(Access::READ_WRITE);
                memcpy(src_data, ite->get()->img_.pixels_, byte_size);
                src.buffer_->unmap();

                composition_cshader_.bind();
                src.bind_image(0, Access::READ_WRITE, ImageUnitFormat::RGBA8UI);
                dst->bind_image(1, Access::READ_WRITE, ImageUnitFormat::RGBA8UI);

                /*********** for debugging ***********/
                void* dbg_ptr = cache_down_.buffer_->map(Access::READ_ONLY);
                cache_down_.buffer_->unmap();
                /*********** for debugging ***********/

                glDispatchCompute(info_.width * info_.height / 16, 1, 1);
                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            }
        }
        ite++;
        count++;
    }
    ite++;  // skip current layer
    // composite cache_up
    count = 0;
    while(1) {
        if (ite == layers_.end()) {
            if (count == 0) {
                void* ptr = cache_up_.buffer_->map(Access::READ_WRITE);
                memset(ptr, 0x00, byte_size);
                cache_up_.buffer_->unmap();
            }
            break;
        } else {
            if (count == 0) {
                void* ptr = cache_up_.buffer_->map(Access::READ_WRITE);
                memcpy(ptr, ite->get()->img_.pixels_, byte_size);
                cache_up_.buffer_->unmap();
            } else {
                // composition
                GLTextureBuffer* dst = &cache_up_;
                GLTextureBuffer src;
                src.init();
                src.allocate(byte_size, bflag_readwrite, internal_format);

                void* src_data = src.buffer_->map(Access::READ_WRITE);
                memcpy(src_data, ite->get()->img_.pixels_, byte_size);
                src.buffer_->unmap();

                composition_cshader_.bind();
                src.bind_image(0, Access::READ_WRITE, ImageUnitFormat::RGBA8UI);
                dst->bind_image(1, Access::READ_WRITE, ImageUnitFormat::RGBA8UI);

                /*********** for debugging ***********/
                void* dbg_ptr = cache_up_.buffer_->map(Access::READ_ONLY);
                cache_up_.buffer_->unmap();
                /*********** for debugging ***********/
                
                glDispatchCompute(info_.width * info_.height / 16, 1, 1);
                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            }
        }
        ite++;
        count++;
    }
}

void Scene::composite_region(RectInt _region) {
    // composite the cache_up and cache_down and current-layer-image to image_
    if (_region.width == 0 || _region.height == 0) return;
    using namespace DGL;
    DLOG_TRACE("update region: pos:(%d, %d) width: %d height: %d",
               _region.posx, _region.posy, _region.width, _region.height);

    if (layers_.size() <= 1) {
        Col_RGBA* ptr_src = (Col_RGBA*)layers_.front().get()->img_.pixels_;
        Col_RGBA* ptr_dst = (Col_RGBA*)image_.pixels_;
        for (size_t i = _region.posy; i < _region.posy + _region.height; i++)
        {
            long src_offset = i * info_.width + _region.posx;
            memcpy(ptr_dst + src_offset, ptr_src  + src_offset, _region.width * sizeof(Col_RGBA));
        }
        return;
    }

    int     byte_size = info_.width * info_.height * sizeof(Col_RGBA);
    BufFlag bflag_read_write = BufFlag::MAP_READ_BIT|BufFlag::MAP_WRITE_BIT|BufFlag::DYNAMIC_STORAGE_BIT;

    // use this texbuffer as dst for all srcs
    GLTextureBuffer tbuf_down;
    tbuf_down.init();
    tbuf_down.allocate(byte_size, bflag_read_write, SizedInternalFormat::RGBA8UI);

    GLTextureBuffer tbuf_layer;
    tbuf_layer.init();
    tbuf_layer.allocate(byte_size, bflag_read_write, SizedInternalFormat::RGBA8UI);

    Col_RGBA* ptr_down  = (Col_RGBA*)tbuf_down.buffer_->map(Access::READ_WRITE);
    Col_RGBA* ptr_layer = (Col_RGBA*)tbuf_layer.buffer_->map(Access::READ_WRITE);

    Col_RGBA* src_down  = (Col_RGBA*)cache_down_.buffer_->map(Access::READ_WRITE);
    Col_RGBA* src_layer = (Col_RGBA*)curr_layer_ite_->get()->img_.pixels_;

    memcpy(ptr_down, src_down, byte_size);
    memcpy(ptr_layer, src_layer, byte_size);

    tbuf_down.buffer_->unmap();
    tbuf_layer.buffer_->unmap();
    cache_down_.buffer_->unmap();

    composition_cshader_.bind();
    tbuf_layer.bind_image(0, Access::READ_WRITE, ImageUnitFormat::RGBA8UI); // src
    tbuf_down.bind_image(1, Access::READ_WRITE, ImageUnitFormat::RGBA8UI);  // dst

    glDispatchCompute(info_.width * info_.height / 16, 1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    cache_up_.bind_image(0, Access::READ_WRITE, ImageUnitFormat::RGBA8UI);
    tbuf_down.bind_image(1, Access::READ_WRITE, ImageUnitFormat::RGBA8UI);

    glDispatchCompute(info_.width * info_.height / 16, 1, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    Col_RGBA* result = (Col_RGBA*)tbuf_down.buffer_->map(Access::READ_WRITE);

    memcpy(image_.pixels_, result, byte_size);
    tbuf_down.buffer_->unmap();
    
    // TODO: here we should have a "region_merge" instead of "region_replace"
    region_ = _region;
}

void Scene::update(RectInt _region) {
    composite_region(_region);
}

void Scene::comfirm_update() {
    memset(&region_, 0, sizeof(RectInt));
}

void Scene::add_layer(Col_RGBA _col) {
    if (layers_.size() == 0) {
        add_layer(_col, "base");
    } else {
        add_layer(_col, "layer" + std::to_string(layers_.size() - 1));
    }
    
    composite_cache();
}
void Scene::add_layer(Col_RGBA _col, const std::string& _name) {
    layers_.emplace_back(std::make_unique<Layer>(
        info_.width,
        info_.height,
        _name,
        _col
    ));
    
    curr_layer_ite_ = --layers_.end();

    composite_cache();
}

void Scene::change_layer(const std::string& _name) {
    // to be done

    composite_cache();
}

void Scene::change_layer(Layer* _layer) {
    for (auto ite = layers_.begin(); ite != layers_.end(); ite++)
    {
        if (ite->get() == _layer) {
            curr_layer_ite_ = ite;
            
            composite_cache();
        }
    }
    assert("you have to input a Layer* that is in the layers_ list");
}

bool Scene::next_layer() {
    LayerIte ite = curr_layer_ite_;
    if (++ite != layers_.end()) {
        curr_layer_ite_ = ite;
        composite_cache();
        return true;
    } else {
        return false;
    }
}
bool Scene::previous_layer() {
    if (curr_layer_ite_ != layers_.begin()) {
        curr_layer_ite_--;
        composite_cache();
        return true;
    }
    return false;
}

Scene::~Scene() {
    // ...
}