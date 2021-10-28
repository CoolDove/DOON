#pragma once
#include <glad/glad.h>
#include <Base/BitMaskEnum.h>
#include "GLEnums.h"
#include <stdint.h>

namespace DGL
{
extern bool gl_inited;

enum class BufFlag : uint32_t {
    DYNAMIC_STORAGE_BIT = 0x0100,
    MAP_READ_BIT        = 0x0001,
    MAP_WRITE_BIT       = 0x0002,
    // MAP_READ_WRITE_BIT  = 0x0003,  // dove defined
    MAP_PERSISTENT_BIT  = 0x0040,  // haven't figure out how to use this
    MAP_COHERENT_BIT    = 0x0080,  // haven't figure out how to use this
    CLIENT_STORAGE_BIT  = 0x0200,
};
BIT_MASK_ENUM(BufFlag)

enum class BufType : uint32_t {
    VERTEX_BUFFER             = 0x8892,
    COPY_READ_BUFFER          = 0x8F36,
    COPY_WRITE_BUFFER         = 0x8F37,
    DRAW_INDIRECT_BUFFER      = 0x8F3F,
    ELEMENT_INDEX_BUFFER      = 0x8893,
    PIXEL_PACK_BUFFER         = 0x88EB,
    PIXEL_UNPACK_BUFFER       = 0x88EC,
    TEXTURE_BUFFER            = 0x8C2A,
    UNIFORM_BUFFER            = 0x8A11,
    TRANSFORM_FEEDBACK_BUFFER = 0x8C8E
};

class Buffer {
public:
    void allocate(size_t _size_b, BufFlag _flag);
    void upload(size_t _size_b, size_t _offset_b, void* _data);

    void bind(BufType _target);
public:
    Buffer();
    ~Buffer();

    void    init();

    void*   map(Access _access);
    void*   map_range(Access _access, long long _offset_b, long long _size_b);
    void    unmap();

    int     get_id()        const { return id_; }
    size_t  get_size()      const { return size_; }
    BufType get_type()      const { return type_; }
    BufFlag get_flag()      const { return flag_; }
    bool    is_mapping()    const { return mapping_; }
private:
    GLuint  id_;
    size_t  size_;
    BufType type_;
    BufFlag flag_; 
    bool    mapping_;
};
}