#pragma once

#include "Base/General.h"
#include "Core/Application.h"
#include "DGLCore/GLTexture.h"
#include <DGLCore/DGLCore.h>
#include <Windows.h>

class Renderer {
public:
    Renderer(Application* _app);
    void init();
    void render();
    void on_ui();
    void recreate_canvas_batch();

    void resize_framebuffer(Dove::IVector2D _size);
public:
    struct OpenGLInfo {
        string version;
        string vendor;
        string renderer;
        string shading_lang_version;
    } gl_info_;
private:
    void set_blend_func();
    void init_opengl();

private:
    Application* app_   = nullptr;

    //-------GL--------
    HDC   device_context_;
    HGLRC gl_context_;

    Program program_canvas_;
    Program program_base_;

    // TODO: GLFrameBuffer class
    GLuint framebuf_;
    DGL::GLTexture2D framebuf_tex_a_;
    DGL::GLTexture2D framebuf_tex_b_;

    GeoBatch batch_;

    GLuint img_id;
};
