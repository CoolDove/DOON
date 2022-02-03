#include "Renderer.h"
#include "Base/General.h"
#include "DGLCore/GLEnums.h"
#include "DGLCore/GLTexture.h"
#include "DoveLog.hpp"

#include <gl/GL.h>
#include <DGLCore/GLDebugger.h>
#include <DGLCore/GLShader.h>
#include <DGLCore/GLFramebuffer.h>

#include "Image.h"
#include "Space.h"
#include "Color.h"

using namespace DGL;


void Renderer::blit(DGL::GLTexture2D* src, DGL::GLTexture2D* dst, Dove::IRect2D rect_src, Dove::IRect2D rect_dst) {
    if (!src || !dst) return;
    
    GLint stash = GLFramebuffer::current_framebuffer();

    GLFramebuffer fbuf[2];
    fbuf[0].init();
    fbuf[1].init();
    fbuf[0].attach(src);
    fbuf[1].attach(dst);

    GLint srcx0 = rect_src.posx;
    GLint srcy0 = rect_src.posy;
    GLint srcx1 = rect_src.posx + rect_src.width;
    GLint srcy1 = rect_src.posy + rect_src.height;
    
    GLint dstx0 = rect_dst.posx;
    GLint dsty0 = rect_dst.posy;
    GLint dstx1 = rect_dst.posx + rect_dst.width;
    GLint dsty1 = rect_dst.posy + rect_dst.height;

    glBlitNamedFramebuffer(
        fbuf[0].get_glid(), fbuf[1].get_glid(),
        srcx0, srcy0, srcx1, srcy1, dstx0, dsty0, dstx1, dsty1,
        GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, stash);
}

Renderer::Renderer(Application *_app)
:   current_paint_tex_(nullptr),
    other_paint_tex_(nullptr),
    scene_(nullptr)
{
    app_ = _app;
    init_opengl();
}

void Renderer::init() {
    batch_.init({{DGL::Attribute::POSITION, 3}, { DGL::Attribute::UV, 2 }});

    int wnd_width = app_->window_info_.width;
    int wnd_height = app_->window_info_.height;

    fbuf_paint_.init();

    realloc_paint_tex();
    recreate_canvas_batch();
}

void Renderer::recreate_canvas_batch() {
    if (app_->curr_scene_) {
        int width  = app_->curr_scene_->info_.width;
        int height = app_->curr_scene_->info_.height;

        batch_.clear();
        batch_.add_quad((float)width, (float)height, "canvas");
        batch_.upload();
    }
}

void Renderer::realloc_paint_tex() {
    if (scene_ == app_->curr_scene_) return;
    if (paint_tex_a_.get_inited())
        paint_tex_a_.release();
    if (paint_tex_b_.get_inited())
        paint_tex_b_.release();

    paint_tex_a_.init();
    paint_tex_a_.allocate(1, SizedInternalFormat::RGBA8,
                             app_->curr_scene_->info_.width, app_->curr_scene_->info_.height,
                             PixFormat::RGBA, PixType::UNSIGNED_BYTE);
    paint_tex_b_.init();
    paint_tex_b_.allocate(1, SizedInternalFormat::RGBA8,
                             app_->curr_scene_->info_.width, app_->curr_scene_->info_.height,
                             PixFormat::RGBA, PixType::UNSIGNED_BYTE);
    scene_ = app_->curr_scene_;
}

void Renderer::render() {
    realloc_paint_tex();
    Scene* scn = app_->curr_scene_;
    Dove::IRect2D updated_region = scn->get_region();

    {// compose layers to paint_tex_
        glDisable(GL_BLEND);

        fbuf_paint_.bind();
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        fbuf_paint_.attach(&paint_tex_a_);
        glClear(GL_COLOR_BUFFER_BIT);
        fbuf_paint_.attach(&paint_tex_b_);
        glClear(GL_COLOR_BUFFER_BIT);
        glDepthFunc(GL_ALWAYS);

        auto paint_shader = app_->RES->GetShader("paint");
        paint_shader->bind();
        glViewport(0, 0, scn->info_.width, scn->info_.height);

        for (auto const& layer : scn->layers_) {
            swap_paint_tex();
            fbuf_paint_.attach(current_paint_tex_);
            glClear(GL_COLOR_BUFFER_BIT);

            paint_shader->uniform_f("_size", (float)scn->info_.width, (float)scn->info_.height);

            (*layer).tex_->bind(0);
            paint_shader->uniform_i("_tex", 0);
            other_paint_tex_->bind(1);
            paint_shader->uniform_i("_paintbuffer", 1);
            batch_.draw_batch();

            // render brush layer
            if (&(*layer) == scn->get_curr_layer()) {
                swap_paint_tex();
                fbuf_paint_.attach(current_paint_tex_);
                glClear(GL_COLOR_BUFFER_BIT);

                scn->brush_layer_.bind(0);
                other_paint_tex_->bind(1);
                batch_.draw_batch();
            }
        }
    }

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int wnd_width  = app_->window_info_.width;
    int wnd_height = app_->window_info_.height;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, wnd_width, wnd_height);

    DGL::Camera* cam = &app_->curr_scene_->camera_;
    glm::mat4 view = Space::mat_world_camera(&app_->curr_scene_->camera_);
    glm::mat4 proj = Space::mat_camproj(&app_->curr_scene_->camera_, wnd_width, wnd_height);

    {// draw base
        auto shader_base = app_->RES->GetShader("base");
        shader_base->bind();
        float cam_size = (10.0f - cam->size_)/10.0f;
        int cell_scale = (int)((cam_size * cam_size * cam_size) * 30 + 1);
        shader_base->uniform_mat("_view", 4, &view[0][0]);
        shader_base->uniform_mat("_proj", 4, &proj[0][0]);
        shader_base->uniform_f("_size", (float)scn->info_.width, (float)scn->info_.height);
        shader_base->uniform_i("_scale", cell_scale);
        batch_.draw_batch();
    }
    
    auto shader_canvas = app_->RES->GetShader("canvas");
    shader_canvas->bind();
    shader_canvas->uniform_mat("_view", 4, &view[0][0]);
    shader_canvas->uniform_mat("_proj", 4, &proj[0][0]);
    if (current_paint_tex_ != nullptr) {// draw canvas
        current_paint_tex_->bind(0);
        shader_canvas->uniform_i("_tex", 0);
        batch_.draw_batch();
    }

}

void Renderer::resize_framebuffer(Dove::IVector2D _size) {
}

void Renderer::init_opengl() {
    device_context_ = GetDC(app_->window_);

    // pixel format
    PIXELFORMATDESCRIPTOR desired_pixel_format = {};
    desired_pixel_format.nSize      = sizeof(desired_pixel_format);
    desired_pixel_format.nVersion   = 1;
    desired_pixel_format.dwFlags    = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
    desired_pixel_format.iPixelType = PFD_TYPE_RGBA;
    desired_pixel_format.cColorBits = 32;
    desired_pixel_format.cAlphaBits = 8;

    PIXELFORMATDESCRIPTOR suggested_pixel_format = {};
    int pixel_format_index = ChoosePixelFormat(device_context_, &desired_pixel_format);

    DescribePixelFormat(device_context_,
                        pixel_format_index,
                        sizeof(suggested_pixel_format),
                        &suggested_pixel_format);

    SetPixelFormat(device_context_, pixel_format_index, &suggested_pixel_format);

    gl_context_ = wglCreateContext(device_context_);
    wglMakeCurrent(device_context_, gl_context_);

    gladLoadGL();

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    SwapBuffers(device_context_);

    gl_info_.version              = (char*)glGetString(GL_VERSION);
    gl_info_.vendor               = (char*)glGetString(GL_VENDOR);
    gl_info_.renderer             = (char*)glGetString(GL_RENDERER);
    gl_info_.shading_lang_version = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    using wglCreateContextAttribsARB_t = 
          HGLRC (WINAPI *) (HDC hDC, HGLRC hshareContext, const int *attribList);

    wglCreateContextAttribsARB_t wglCreateContextAttribsARB
        = (wglCreateContextAttribsARB_t)wglGetProcAddress("wglCreateContextAttribsARB");
    if (wglCreateContextAttribsARB) {
        HGLRC share_context = 0;
        int attrib_list[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
            WGL_CONTEXT_MINOR_VERSION_ARB, 5,
#ifdef DEBUG
            WGL_CONTEXT_FLAGS_ARB,         WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
            WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };
        HGLRC modern_glrc = wglCreateContextAttribsARB(device_context_, share_context, attrib_list);
        if (modern_glrc) {
            // switch to modern openGL context
            wglMakeCurrent(device_context_, modern_glrc);
            gl_context_ = modern_glrc;
        }
#ifdef DEBUG
        gl_debug_init();
#endif
    }
}

void Renderer::on_ui() {

}
