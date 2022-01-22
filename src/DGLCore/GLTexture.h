#pragma once
#include <glad/glad.h>
#include <stdint.h>
#include "GLEnums.h"
#include "GLBuffer.h"
#include <assert.h>
#include <memory>

// TODO: immutable textures and mutable textures
namespace DGL
{
enum class TexType : uint32_t {
    TEXTURE_1D                   = 0x0DE0,
    TEXTURE_2D                   = 0x0DE1,
    TEXTURE_3D                   = 0x806F,
    TEXTURE_1D_ARRAY             = 0x8C18,
    TEXTURE_2D_ARRAY             = 0x8C1A,
    TEXTURE_RECTANGLE            = 0x84F5,
    TEXTURE_CUBE_MAP             = 0x8513,
    TEXTURE_CUBE_MAP_ARRAY       = 0x9009,
    TEXTURE_BUFFER               = 0x8C2A,
    TEXTURE_2D_MULTISAMPLE       = 0x9100,
    TEXTURE_2D_MULTISAMPLE_ARRAY = 0x9102,
    UNKNOWN                      = 0x0000,
};

enum class PixFormat : uint32_t {
    RED             = 0x1903,
    RG              = 0x8227,
    RGB             = 0x1907,
    BGR             = 0x80E0,

    RGBA            = 0x1908,
    BGRA            = 0x80E1,

    DEPTH_COMPONENT = 0x1902,
    STENCIL_INDEX   = 0x1901,
};

enum class PixType : uint32_t {
    UNSIGNED_BYTE               = 0x1401,
    BYTE                        = 0x1400,

    UNSIGNED_SHORT              = 0x1403,
    SHORT                       = 0x1402,

    UNSIGNED_INT                = 0x1405,
    INT                         = 0x1404,

    FLOAT                       = 0x1406,

    UNSIGNED_BYTE_3_3_2         = 0x8032,
    UNSIGNED_BYTE_2_3_3_REV     = 0x8362,

    UNSIGNED_SHORT_5_6_5        = 0x8363,
    UNSIGNED_SHORT_5_6_5_REV    = 0x8364,

    UNSIGNED_SHORT_4_4_4_4      = 0x8033,
    UNSIGNED_SHORT_4_4_4_4_REV  = 0x8365,

    UNSIGNED_SHORT_5_5_5_1      = 0x8034,
    UNSIGNED_SHORT_1_5_5_5_REV  = 0x8366,

    UNSIGNED_INT_8_8_8_8        = 0x8035,
    UNSIGNED_INT_8_8_8_8_REV    = 0x8367,

    UNSIGNED_INT_10_10_10_2     = 0x8036,
    UNSIGNED_INT_2_10_10_10_REV = 0x8368,
};

enum class TexParam : uint32_t {
    TEXTURE_MIN_FILTER = 0x2801,
    TEXTURE_MAG_FILTER = 0x2800,
    TEXTURE_WRAP_S     = 0x2802,
    TEXTURE_WRAP_T     = 0x2803,
    TEXTURE_WRAP_R     = 0x8072,
};

enum class TexFilter : uint32_t {
    NEAREST = 0x2600,
    LINEAR  = 0x2601,
};

enum class TexWrap : uint32_t {
    CLAMP_TO_EDGE        = 0x812F,
    CLAMP_TO_BORDER      = 0x812D,
    MIRRORED_REPEAT      = 0x8370,
    REPEAT               = 0x2901,
    MIRROR_CLAMP_TO_EDGE = 0x8743,
};

class GLTexture2D {
public:
    GLTexture2D();
    ~GLTexture2D();

    GLTexture2D(const GLTexture2D&) = delete;
    GLTexture2D* operator=(const GLTexture2D&) = delete;
    
    void init(bool _immutable = true);
    void release();

public:
    TexType  get_type() const { return type_; }
    uint32_t get_glid() const { return id_; }
    bool     get_inited() const { return inited_; }
    bool     get_allocated() const { return allocated_; }

    // params in the second line are used only when this is a mutable texture
    void allocate(uint32_t _levels, SizedInternalFormat _format, int _width, int _height,
                  PixFormat _pix_format = PixFormat::BGRA, PixType _pix_type = PixType::UNSIGNED_BYTE, void* _data = nullptr);
    // TODO: upload region in texture class
    // void upload_region(uint32_t _level, Dove::IRect2D _region, Dove::IVector2D _src_info, ...);
    void upload(uint32_t _level, int _offset_x, int _offset_y,
                int _width, int _height, PixFormat _format, PixType _type, void* _data);
    void bind(uint32_t _unit);
    void bind_image(uint32_t _unit, uint32_t _level, bool _layered, int _layer,
                    Access _acc, ImageUnitFormat _format);

    uint32_t get_levels_count() const { return levels_count_; }
private:
    uint32_t levels_count_;

public:
    void param_min_filter(TexFilter _filter) {
        assert(inited_);
        glTextureParameteri(id_, (GLenum)TexParam::TEXTURE_MIN_FILTER, (GLenum)_filter);
    }

    void param_mag_filter(TexFilter _filter) {
        assert(inited_);
        glTextureParameteri(id_, (GLenum)TexParam::TEXTURE_MAG_FILTER, (GLenum)_filter);
    }

    void param_wrap_r(TexWrap _wrap) {
        assert(inited_);
        glTextureParameteri(id_, (GLenum)TexParam::TEXTURE_WRAP_R, (GLenum)_wrap);
    }
    void param_wrap_s(TexWrap _wrap) {
        assert(inited_);
        glTextureParameteri(id_, (GLenum)TexParam::TEXTURE_WRAP_S, (GLenum)_wrap);
    }

    void param_wrap_t(TexWrap _wrap) {
        assert(inited_);
        glTextureParameteri(id_, (GLenum)TexParam::TEXTURE_WRAP_T, (GLenum)_wrap);
    }

public:
    struct {
        uint32_t width;
        uint32_t height;
        uint32_t levels;
        SizedInternalFormat format;
    } info_;

private:
    GLuint  id_;
    bool    inited_;
    bool    immutable_;
    bool    allocated_;
    TexType type_;
};

}
