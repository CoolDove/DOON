#pragma once

#include "Core/Application.h"
#include <DGLCore/DGLCore.h>
#include <Windows.h>

class Renderer {
public:
    Renderer(Application* _app);
    void init();
    void render();
    void on_ui();
    void create_gl_image();

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

    DGL::Program program_canvas_;
    DGL::Program program_base_;

    DGL::GLTexture2D tex_img_;
    DGL::GeoBatch batch_;

    GLuint img_id;
};