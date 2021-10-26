#include "GLTexture.h"
#include "GLDebugger.h"

namespace DGL
{
GLTexture::GLTexture()
:   id_(0),
    inited_(false),
    type_(TexType::UNKNOWN)
{
}
GLTexture::~GLTexture() {
    if (inited_) 
        glDeleteTextures(1, &id_);
}

void GLTexture::init() {
    assert(!inited_);
    glCreateTextures((GLenum)type_, 1, &id_);

    if (!id_) throw DGL::EXCEPTION::CREATION_FAILED();
    inited_ = true;
}

void GLTexture::release() {
    if (inited_) glDeleteTextures(1, &id_);
    inited_ = false;
}

/*──────────┐
│ texture2D │
└──────────*/
GLTexture2D::GLTexture2D()
:   GLTexture()
{
    type_ = TexType::TEXTURE_2D;
}

void GLTexture2D::allocate(uint32_t _levels, SizedInternalFormat _format, int _width, int _height) {
    assert(inited_ && "texture haven't been initialized");
    assert(_levels > 0 && "_levels should be at least '1'");
    glTextureStorage2D(id_, _levels, (GLenum)_format, _width, _height);
    levels_count_ = _levels;
}

void GLTexture2D::upload(uint32_t _level, int _offset_x, int _offset_y,
                         int _width, int _height, PixFormat _format, PixType _type, void* _data)
{
    assert(inited_);
    glTextureSubImage2D(id_, _level, _offset_x, _offset_y, _width, _height,
                        (GLenum)_format, (GLenum)_type, _data);
}

void GLTexture2D::bind(uint32_t _unit) {
    assert(inited_);
    glBindTextureUnit(_unit, id_);
}

void GLTexture2D::bind_image(uint32_t _unit, uint32_t _level, bool _layered, int _layer,
                             Access _acc, ImageUnitFormat _format)
{
    assert(inited_);
    glBindImageTexture(_unit, id_, _level, _layered, _layer, (GLenum)_acc, (GLenum)_format);
}


/*───────────────┐
│ texture buffer │
└───────────────*/
GLTextureBuffer::GLTextureBuffer()
:   GLTexture(),
    attached_(false)
{
    type_ = TexType::TEXTURE_BUFFER;
}

void GLTextureBuffer::init() {
    assert(!inited_);
    glCreateTextures((GLenum)type_, 1, &id_);

    if (!id_) throw DGL::EXCEPTION::CREATION_FAILED();
    inited_ = true;
}

void GLTextureBuffer::allocate(size_t _size_b, BufFlag _flag, SizedInternalFormat _format) {
    assert(inited_);

    if (!attached_) {
        buffer_ = std::make_unique<Buffer>();
        buffer_->init();
        buffer_->allocate(_size_b, _flag);
        glTextureBuffer(id_, (GLenum)_format, buffer_->get_id());
        attached_ = true;
    } else {
        buffer_->allocate(_size_b, _flag);
    }
}

void GLTextureBuffer::bind_image(uint32_t _unit, Access _acc, ImageUnitFormat _format) {
    assert(inited_&&attached_);
    glBindImageTexture(_unit, id_, 0, false, 0, (GLenum)_acc, (GLenum)_format);
}

}