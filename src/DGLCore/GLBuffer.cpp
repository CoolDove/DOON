﻿#include "GLBuffer.h"
#include "DoveLog.hpp"
#include <assert.h>
#include "DGLBase.h"

namespace DGL
{

Buffer::Buffer() 
:	id_(0),
    size_(0)
{
}

Buffer::~Buffer() {
    glDeleteBuffers(1, &id_);
}

void Buffer::init() {
    glCreateBuffers(1, &id_);
}

void Buffer::allocate(size_t _size, BufFlag _flag) {
    glNamedBufferStorage(id_, 2 * _size, nullptr, static_cast<GLuint>(_flag));

    flag_ = _flag;
    size_ = _size;
}

void Buffer::upload(size_t _size, size_t _offset, void* _data) {
    assert(_size + _offset <= size_ && " the data size is larger than the buffer size ");
    glNamedBufferSubData(id_, _offset, _size, _data);
}

void Buffer::bind(BufType _target) {
    glBindBuffer(static_cast<GLuint>(_target), id_);
    type_ = _target;
}

void* Buffer::map(MapAcc _access) {
    if (_access == MapAcc::READ_ONLY)
        assert((uint32_t)(flag_&BufFlag::MAP_READ_BIT)&&"the buffer doesn't have MAP_READ_BIT on");
    else if (_access == MapAcc::WRITE_ONLY)
        assert((uint32_t)(flag_&BufFlag::MAP_WRITE_BIT)&&"the buffer doesn't have MAP_WRITE_BIT on");
    else if (_access == MapAcc::READ_WRITE)
        assert((uint32_t)(flag_&BufFlag::MAP_WRITE_BIT)&&(uint32_t)(flag_&BufFlag::MAP_READ_BIT)&&"the buffer doesn't have MAP_WRITE_BIT MAP_READ_BIT and on");

    void* ptr = glMapNamedBuffer(id_, (GLenum)_access);
    if (!ptr) throw DGLERROR::BUFFER_MAPPING_FAILED;
    mapping_ = true;
    return ptr;
}

void* Buffer::map_range(MapAcc _access, long long _offset, long long _size) {
    if (_access == MapAcc::READ_ONLY)
        assert((uint32_t)(flag_&BufFlag::MAP_READ_BIT)&&"the buffer doesn't have MAP_READ_BIT on");
    else if (_access == MapAcc::WRITE_ONLY)
        assert((uint32_t)(flag_&BufFlag::MAP_WRITE_BIT)&&"the buffer doesn't have MAP_WRITE_BIT on");
    else if (_access == MapAcc::READ_WRITE)
        assert((uint32_t)(flag_&BufFlag::MAP_WRITE_BIT)&&(uint32_t)(flag_&BufFlag::MAP_READ_BIT)&&"the buffer doesn't have MAP_WRITE_BIT MAP_READ_BIT and on");

    void* ptr = glMapNamedBufferRange(id_, _offset, _size, (GLenum)_access);
    if (!ptr) throw DGLERROR::BUFFER_MAPPING_FAILED;
    mapping_ = false;
    return ptr;
}
}