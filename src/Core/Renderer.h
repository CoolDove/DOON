#pragma once
#include "Core/Application.h"
#include <Windows.h>
#include <DGLCore/DGLCore.h>

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
    Application* app_   = nullptr;

private://GL
    HDC   device_context_;
    HGLRC gl_context_;

    DGL::Program program_canvas_;
    DGL::Program program_base_;

    DGL::GLTexture2D tex_img_;
    DGL::GeoBatch batch_;

    GLuint img_id;
private:// private func
    void init_opengl();
};