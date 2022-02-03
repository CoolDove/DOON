#pragma once

#include "Base/General.h"
#include "Core/Application.h"
#include "DGLCore/GLGeoBatch.h"
#include "DGLCore/GLProgram.h"
#include "DGLCore/GLTexture.h"
#include <DGLCore/DGLCore.h>
#include <DGLCore/GLFramebuffer.h>
#include <Windows.h>

class Renderer {
public:
    static void blit(DGL::GLTexture2D* src, DGL::GLTexture2D* dst, Dove::IRect2D rect_src, Dove::IRect2D rect_dst);
public:
    Renderer(Application* _app);
    void init();
    void render();
    void on_ui();
    void recreate_canvas_batch();

    void resize_framebuffer(Dove::IVector2D _size);

    GeoBatch* get_canvas_quad() { return &batch_; };

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

    GLFramebuffer fbuf_paint_;
    DGL::GLTexture2D paint_tex_a_;
    DGL::GLTexture2D paint_tex_b_;
    DGL::GLTexture2D* current_paint_tex_;
    DGL::GLTexture2D* other_paint_tex_;

    Scene* scene_;
    void realloc_paint_tex();

    void swap_paint_tex() {
        if (current_paint_tex_ == nullptr) {
            current_paint_tex_ = &paint_tex_a_;
            other_paint_tex_ = &paint_tex_b_;
        }
        else if (current_paint_tex_ == &paint_tex_a_) {
            current_paint_tex_ = &paint_tex_b_;
            other_paint_tex_ = &paint_tex_a_;
        } 
        else if (current_paint_tex_ == &paint_tex_b_) {
            current_paint_tex_ = &paint_tex_a_;
            other_paint_tex_ = &paint_tex_b_;
        }
    }

    GeoBatch batch_;

    GLuint img_id;
};
