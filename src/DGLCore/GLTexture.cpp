#include "GLTexture.h"
#include "GLDebugger.h"

namespace DGL
{
GLTexture2D::GLTexture2D()
:   id_(0),
    inited_(false),
    type_(TexType::UNKNOWN),
    info_({0}),
    allocated_(false)
{
    type_ = TexType::TEXTURE_2D;

}

GLTexture2D::~GLTexture2D() {
    if (inited_) 
        glDeleteTextures(1, &id_);
}

void GLTexture2D::init(bool _immutable) {
    assert(!inited_);
    glCreateTextures((GLenum)type_, 1, &id_);
    if (!id_) throw DGL::EXCEPTION::CREATION_FAILED();
    inited_ = true;
}

void GLTexture2D::release() {
    if (inited_) glDeleteTextures(1, &id_);
    inited_ = false;
    allocated_ = false;
}

void GLTexture2D::allocate(uint32_t _levels, SizedInternalFormat _format,
                           int _width, int _height,
                           PixFormat _pix_format, PixType _pix_type, void* data)
{
    assert(inited_ && "texture haven't been initialized");
    assert(_levels > 0 && "_levels should be at least '1'");
    assert((_width > 0 && _height > 0 && _levels > 0) && "GLTexture level and width and height must be greater than 0");

    if (immutable_) {
        assert(!allocated_ && "cannot reallocate an immutable GLTexture2D");
        glTextureStorage2D(id_, _levels, (GLenum)_format, _width, _height);
        levels_count_ = _levels;
    } else {
        glBindTexture(GL_TEXTURE_2D, id_);
        glTexImage2D(GL_TEXTURE_2D, _levels, (GLenum)_format,
                     _width, _height, 0,
                     (GLenum)_pix_format, (GLenum)_pix_type,
                     data);
    }

    info_.width = _width;
    info_.height = _height;
    info_.levels = _levels;
    info_.format = _format;

    allocated_ = true;
}

void GLTexture2D::upload(uint32_t _level, int _offset_x, int _offset_y,
                         int _width, int _height, PixFormat _format, PixType _type, void* _data)
{
    assert(inited_ && "GLTexture has not been initialized and allocated, but you're trying to upload to it");
    assert(allocated_ && "should allocate a GLTexture before uploading");

    if (immutable_) {
        glTextureSubImage2D(id_, _level, _offset_x, _offset_y, _width, _height,
                           (GLenum)_format, (GLenum)_type, _data);
    } else {
        glBindTexture(GL_TEXTURE_2D, id_);
        glTexImage2D(GL_TEXTURE_2D, info_.levels, (GLenum)info_.format, info_.width, info_.height,
                     0, (GLenum)_format, (GLenum)_type, _data);
    }
}

void GLTexture2D::bind(uint32_t _unit) {
    assert(inited_);
    glBindTextureUnit(_unit, id_);
}

void GLTexture2D::bind_image(
    uint32_t _unit, uint32_t _level, bool _layered, int _layer, Access _acc, ImageUnitFormat _format)
{
    assert(inited_);
    glBindImageTexture(_unit, id_, _level, _layered, _layer, (GLenum)_acc, (GLenum)_format);
}

}
