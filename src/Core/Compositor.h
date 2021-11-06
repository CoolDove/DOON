#pragma once
#include "Core/Color.h"
#include <stdint.h>
#include <unordered_map>
#include <DGLCore/GLProgram.h>
#include <DGLCore/GLTexture.h>
#include <vector>
#include <list>
#include <thread>
#include <mutex>

class ComposeBuffer {
public:
    ComposeBuffer(uint32_t _size_b);
    ~ComposeBuffer();
    ComposeBuffer(const ComposeBuffer&) = delete;
    ComposeBuffer& operator=(const ComposeBuffer&) = delete;

    void realloc(uint32_t _size_b);

    uint32_t get_size() const { return size_; }

    DGL::GLTextureBuffer tex_buffer_;
    bool     occupying_;
private:
    uint32_t size_;
};

class Compositor {
private:
    using CompShaderMap = std::unordered_map<std::string, DGL::Program>;
    using BufferList    = std::list<ComposeBuffer>;
public:
    const uint32_t MAX_BUFFERS_NUM = 16;
    const uint32_t GROUP_SIZE_X = 16;

    Compositor();
    ~Compositor();

    void load_shaders();

    // NOTE: this function returns an id, after glsync(), you can use that id to get the result
    void compose(const std::string& _composition_func,
                 DGL::GLTextureBuffer* _src, DGL::GLTextureBuffer* _dst,
                 uint32_t _size_b, bool _sync = true);

    uint32_t compose(const std::string& _composition_func,
                     Col_RGBA* _src, Col_RGBA* _dst,
                     uint32_t _size_b, bool _sync = true);
    void glsync();
    void get_result(uint32_t _id, Col_RGBA* _buffer, uint32_t _size_b, bool _release = true);
    void release_result_buffer(uint32_t _id);
private:
    void add_compshader(std::string _name, std::string _load_path);
    void add_buffer(uint32_t _size_b);
    ComposeBuffer* get_buffer(uint32_t _size_b, bool _occupy = true);
    ComposeBuffer* find_buffer(uint32_t _id);
    void release_result_buffer(ComposeBuffer* _buffer);

    CompShaderMap comp_shaders_;
    BufferList    buffers_;
    std::mutex    buffer_mt_;
};
