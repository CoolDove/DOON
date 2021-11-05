#include "Compositor.h"
#include "DGLCore/GLBuffer.h"
#include "DGLCore/GLEnums.h"
#include <stdint.h>

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
    comp_shaders_["default"].init();
}

Compositor::~Compositor() {
}

void Compositor::load_shaders() {
  
}

uint32_t Compositor::compose(Col_RGBA *_src, Col_RGBA *_dst, Col_RGBA *_result,
                             uint32_t _size_b, bool _sync)
{
    // TODO: finish this
    return 0;
}

void Compositor::get_result(uint32_t _id, Col_RGBA *_buffer) {

}
void Compositor::glsync() {
  
}

void Compositor::add_buffer(uint32_t _size_b) {
    buffer_mt_.lock();
    buffers_.emplace_back(_size_b);
    buffer_mt_.unlock();
}

ComposeBuffer* Compositor::get_buffer(uint32_t _size_b) {
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

            if (dist < distance) {
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
            // NOTE:
            // do nothing so that the function returns nullptr
            // maybe someday we could wait for glsync here and call get_buffe() recursively again.
            // but that is dangerous
          }
        } else {
            add_buffer(_size_b);
            target = &buffers_.back();
        }
    }
    return target;
}

void Compositor::add_compshader(std::string _name, std::string _load_path) {

}

