#pragma once
#include "Core/Application.h"
// #include "Core/Image.h"
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
    Application* app_   = nullptr;

private://GL
    HDC   device_context_;
    HGLRC gl_context_;

    DGL::Shader   shader_canvas_;
    DGL::Shader   shader_base_;

    DGL::GeoBatch batch_;
    // DGL::GeoBatch base_;// we dont need this actually, just render the batch_ twice

    GLuint img_id;
private:// private func
    void init_opengl();
};