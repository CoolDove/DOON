#include "Compositor.h"
#include "DGLCore/GLBuffer.h"
#include "DGLCore/GLEnums.h"
#include "DGLCore/GLProgram.h"
#include "DGLCore/GLShader.h"
#include "DGLCore/GLTexture.h"
#include "DoveLog.hpp"
#include <DGLCore/GLDebugger.h>
#include <stdint.h>
#include <string.h>

ComposeBuffer::ComposeBuffer(uint32_t _size_b)
:   size_(_size_b),
    occupying_(false)      
{
    using namespace DGL;
    tex_buffer_.init();
    BufFlag flags = BufFlag::DYNAMIC_STORAGE_BIT | BufFlag::MAP_WRITE_BIT | BufFlag::MAP_READ_BIT;
    tex_buffer_.allocate(_size_b, flags, SizedInternalFormat::RGBA8UI);
}

void ComposeBuffer::realloc(uint32_t _size_b) {
    if (_size_b < size_) return;
    using namespace DGL;
    BufFlag flags = BufFlag::DYNAMIC_STORAGE_BIT | BufFlag::MAP_WRITE_BIT | BufFlag::MAP_READ_BIT;
    tex_buffer_.allocate(_size_b, flags, SizedInternalFormat::RGBA8UI);
    size_ = _size_b;
}

ComposeBuffer::~ComposeBuffer() {
}

Compositor::Compositor() {
    add_compshader("common", "./res/shaders/blend_common.comp");
}

Compositor::~Compositor() {
}

void Compositor::load_shaders() {
  
}

void Compositor::compose(const std::string& _composition_func,
                         DGL::GLTextureBuffer* _src, DGL::GLTextureBuffer* _dst,
                         uint32_t _size_b, bool _sync)
{
    using namespace DGL;
    comp_shaders_[_composition_func].bind();
    _src->bind_image(0, Access::READ_WRITE, ImageUnitFormat::RGBA8UI);
    _dst->bind_image(1, Access::READ_WRITE, ImageUnitFormat::RGBA8UI);
    glDispatchCompute((_size_b / sizeof(Col_RGBA)) / GROUP_SIZE_X, 1, 1);
    if (_sync) glsync();
}

uint32_t Compositor::compose(const std::string &_composition_func,
                             Col_RGBA *_src, Col_RGBA *_dst,
                             uint32_t _size_b, bool _sync)
{
    using namespace DGL;
    ComposeBuffer* src_buffer = get_buffer(_size_b);
    ComposeBuffer* dst_buffer = get_buffer(_size_b);
    assert(src_buffer && dst_buffer);// they shouldnt be nullptr for now
    src_buffer->occupying_ = true;
    dst_buffer->occupying_ = true;

    Col_RGBA* ptr_src = (Col_RGBA*)src_buffer->tex_buffer_.buffer_->map(Access::READ_WRITE);    
    Col_RGBA* ptr_dst = (Col_RGBA*)dst_buffer->tex_buffer_.buffer_->map(Access::READ_WRITE);

    memcpy(ptr_src, _src, _size_b);
    memcpy(ptr_dst, _dst, _size_b);

    src_buffer->tex_buffer_.buffer_->unmap();
    dst_buffer->tex_buffer_.buffer_->unmap();

    assert(comp_shaders_.find(_composition_func) != comp_shaders_.end());

    comp_shaders_[_composition_func].bind();
    src_buffer->tex_buffer_.bind_image(0, Access::READ_WRITE, ImageUnitFormat::RGBA8UI);// 0 means src
    dst_buffer->tex_buffer_.bind_image(1, Access::READ_WRITE, ImageUnitFormat::RGBA8UI);// 1 means dst

    glDispatchCompute((_size_b / sizeof(Col_RGBA)) / GROUP_SIZE_X, 1, 1);

    if (_sync) glsync();

    src_buffer->occupying_ = false;
    return dst_buffer->tex_buffer_.get_glid();
}

void Compositor::get_result(uint32_t _id, Col_RGBA *_buffer, uint32_t _size_b, bool _release) {
    ComposeBuffer* buf = find_buffer(_id);
    if (buf && buf->occupying_) {
        memcpy(_buffer, buf->tex_buffer_.buffer_->map(DGL::Access::READ_WRITE), _size_b);
        buf->tex_buffer_.buffer_->unmap();

        if (_release) release_result_buffer(buf);
    }
}

void Compositor::release_result_buffer(uint32_t _id) {
    release_result_buffer(find_buffer(_id));
}

void Compositor::release_result_buffer(ComposeBuffer* _buffer) {
    if (_buffer) _buffer->occupying_ = false;
}

void Compositor::glsync() {
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Compositor::add_buffer(uint32_t _size_b) {
    buffer_mt_.lock();
    buffers_.emplace_back(_size_b);
    buffer_mt_.unlock();
}

ComposeBuffer *Compositor::find_buffer(uint32_t _id) {
    for (auto ite = buffers_.begin(); ite != buffers_.end(); ite++) {
        if (ite->tex_buffer_.get_glid() == _id) {
            return &(*ite);
        }
    }
    return nullptr;
}

ComposeBuffer* Compositor::get_buffer(uint32_t _size_b, bool _occupy) {
    ComposeBuffer* target = nullptr;
    ComposeBuffer* closet_target = nullptr;
    uint32_t distance = 0xffffffff;
    
    bool need_closet = true;
    for (auto ite = buffers_.begin(); ite != buffers_.end(); ite++) {
        if (ite->get_size() >= _size_b && !ite->occupying_) {
            if (!target) {
                target = &(*ite); 
                need_closet = false;
            } else if (target->get_size() > ite->get_size()) {
                target = &(*ite);
            }
        } else if (need_closet) {
            int dist = (int)_size_b - (int)ite->get_size() ;
            if (dist < 0) dist = -dist;

            if ((uint32_t)dist < distance) {
                closet_target = &(*ite);
                distance = (uint32_t)dist;
            }
        }
    }
    if (!target) {
        if (buffers_.size() == MAX_BUFFERS_NUM) {
          if (closet_target) {
              closet_target->realloc(_size_b);
          } else {
            // FIXME:
            // do nothing so that the function returns nullptr
            // maybe someday we could wait for glsync here and call get_buffe() recursively again.
            // but that is dangerous
            // @temp:
            assert(0 && " there is no more buffer to be used ");
          }
        } else {
            add_buffer(_size_b);
            target = &buffers_.back();
        }
    }
    if (target && _occupy) target->occupying_ = true;
    return target;
}

void Compositor::add_compshader(std::string _name, std::string _load_path) {
    using namespace DGL;
    Shader shader;
    shader.init(ShaderType::COMPUTE_SHADER);

    try {
        shader.load(_load_path);
    } catch (const EXCEPTION::DGLException& err) {
        DLOG_ERROR("failed to load shader: %s", err.msg.c_str());
        assert(0&& " file not exist ");
    }

    // try {
        // shader.load(_load_path);
    // } catch (const EXCEPTION::SHADER_COMPILING_FAILED& exp) {
        // DLOG_ERROR("ShaderError: %s", exp.msg.c_str());
        // assert(0&& " failed to compile compute shader ");
    // } catch (const EXCEPTION::FILE_NOT_EXIST& exp) {
        // assert(0&& " file not exist ");
    // }

    comp_shaders_[_name].init();
    comp_shaders_[_name].link(1, &shader);
}

