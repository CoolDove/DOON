#include "GLFramebuffer.h"


namespace DGL
{

GLFramebuffer::GLFramebuffer() {
}

GLFramebuffer::~GLFramebuffer() {
    if (inited_) glDeleteFramebuffers(1, &id_);
}

void GLFramebuffer::init() {
    glCreateFramebuffers(1, &id_);
    inited_ = true;
}

void GLFramebuffer::bind(FramebufferType _type) {
    if (!inited_) {
        DLOG_ERROR("binding framebuffer failed because it's not inited");
        return;
    }
    glBindFramebuffer((GLenum)_type, id_);
}

GLint GLFramebuffer::current_framebuffer(FramebufferType _type) {
    GLenum type;
    switch (_type) {
        case FramebufferType::FRAMEBUFFER: type = GL_FRAMEBUFFER_BINDING; break;
        case FramebufferType::DRAW_FRAMEBUFFER: type = GL_DRAW_FRAMEBUFFER_BINDING; break;
        case FramebufferType::READ_FRAMEBUFFER: type = GL_READ_FRAMEBUFFER_BINDING; break;
    }

    GLint get;
    glGetIntegerv(type, &get);
    return get;
}

void GLFramebuffer::attach(GLTexture2D* tex, FramebufferAttachment attachment, int texlayer) {
    if (!inited_) {
        DLOG_ERROR("attach framebuffer failed because it's not inited");
        return;
    }
    if (tex != nullptr && tex->get_glid()) {
        glNamedFramebufferTexture(id_, (GLenum)attachment, tex->get_glid(), texlayer);
    }
}

}    
