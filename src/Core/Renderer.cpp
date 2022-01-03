#include "Renderer.h"
#include "Base/General.h"
#include "DGLCore/GLEnums.h"
#include "DGLCore/GLTexture.h"
#include "DoveLog.hpp"

#include <gl/GL.h>
#include <DGLCore/GLDebugger.h>
#include <DGLCore/GLShader.h>

#include "Image.h"
#include "Space.h"
#include "Color.h"

using namespace DGL;
Renderer::Renderer(Application *_app)
:   framebuf_(0),
    current_paint_tex_(nullptr),
    other_paint_tex_(nullptr)
{
    app_ = _app;
    init_opengl();
}

void Renderer::init() {
    batch_.init({{DGL::Attribute::POSITION, 3}, { DGL::Attribute::UV, 2 }});

    try {
        Shader canvas_vert("./res/shaders/canvas.vert", ShaderType::VERTEX_SHADER);
        Shader canvas_frag("./res/shaders/canvas.frag", ShaderType::FRAGMENT_SHADER);
        program_canvas_.link(2, &canvas_vert, &canvas_frag);
    } catch (const DGL::EXCEPTION::SHADER_COMPILING_FAILED& err) {
        DLOG_ERROR("shader error: %s", err.msg.c_str());
    }

    try {
        Shader base_vert("./res/shaders/base.vert", ShaderType::VERTEX_SHADER);
        Shader base_frag("./res/shaders/base.frag", ShaderType::FRAGMENT_SHADER);
        program_base_.link(2, &base_vert, &base_frag);
    } catch (const DGL::EXCEPTION::SHADER_COMPILING_FAILED& err) {
        DLOG_ERROR("shader error: %s", err.msg.c_str());
    }

    try {
        Shader paint_vert("./res/shaders/paint.vert", ShaderType::VERTEX_SHADER);
        Shader paint_frag("./res/shaders/paint.frag", ShaderType::FRAGMENT_SHADER);
        program_paint_.link(2, &paint_vert, &paint_frag);
    } catch (const DGL::EXCEPTION::SHADER_COMPILING_FAILED& err) {
        DLOG_ERROR("shader error: %s", err.msg.c_str());
    }

    // create framebuffer and framebuffer textures
    int wnd_width = app_->window_info_.width;
    int wnd_height = app_->window_info_.height;

    framebuf_tex_a_.init();
    framebuf_tex_a_.allocate(1, SizedInternalFormat::RGBA8,
                           wnd_width, wnd_height,
                           PixFormat::RGBA, PixType::UNSIGNED_BYTE);
    framebuf_tex_b_.init();
    framebuf_tex_b_.allocate(1, SizedInternalFormat::RGBA8,
                           wnd_width, wnd_height,
                           PixFormat::RGBA, PixType::UNSIGNED_BYTE);

    paint_tex_a_.init();
    paint_tex_a_.allocate(1, SizedInternalFormat::RGBA8,
                             app_->curr_scene_->info_.width, app_->curr_scene_->info_.height,
                             PixFormat::RGBA, PixType::UNSIGNED_BYTE);
    paint_tex_b_.init();
    paint_tex_b_.allocate(1, SizedInternalFormat::RGBA8,
                             app_->curr_scene_->info_.width, app_->curr_scene_->info_.height,
                             PixFormat::RGBA, PixType::UNSIGNED_BYTE);

    glCreateFramebuffers(1, &framebuf_);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuf_);

    glCreateFramebuffers(2, &fbuf_layers_);
    // glBindFramebuffer(GL_FRAMEBUFFER, fbuf_layers_);

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

// FIXME: **blending error
void Renderer::render() {

    Scene* scn = app_->curr_scene_;
    Dove::IRect2D updated_region = scn->get_region();

    {// compose layers to paint_tex_
        glDisable(GL_BLEND);
        glBindFramebuffer(GL_FRAMEBUFFER, fbuf_layers_);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glNamedFramebufferTexture(fbuf_layers_, GL_COLOR_ATTACHMENT0, paint_tex_b_.get_glid(), 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glNamedFramebufferTexture(fbuf_layers_, GL_COLOR_ATTACHMENT0, paint_tex_a_.get_glid(), 0);
        glClear(GL_COLOR_BUFFER_BIT);

        program_paint_.bind();
        glViewport(0, 0, scn->info_.width, scn->info_.height);

        for (auto const& layer : scn->layers_) {
            swap_paint_tex();
            glNamedFramebufferTexture(fbuf_layers_, GL_COLOR_ATTACHMENT0, current_paint_tex_->get_glid(), 0);
            glClear(GL_COLOR_BUFFER_BIT);

            program_paint_.uniform_f("_size", (float)scn->info_.width, (float)scn->info_.height);

            (*layer).tex_->bind(0);
            program_paint_.uniform_i("_tex", 0);
            other_paint_tex_->bind(1);
            program_paint_.uniform_i("_paintbuffer", 1);
            batch_.draw_batch();

            // render brush layer
            if (&(*layer) == scn->get_curr_layer()) {
                swap_paint_tex();
                glNamedFramebufferTexture(fbuf_layers_, GL_COLOR_ATTACHMENT0, current_paint_tex_->get_glid(), 0);
                glClear(GL_COLOR_BUFFER_BIT);

                scn->brush_layer_->tex_->bind(0);
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
        program_base_.bind();
        float cam_size = (10.0f - cam->size_)/10.0f;
        int cell_scale = (int)((cam_size * cam_size * cam_size) * 30 + 1);
        program_base_.uniform_mat("_view", 4, &view[0][0]);
        program_base_.uniform_mat("_proj", 4, &proj[0][0]);
        program_base_.uniform_f("_size", (float)scn->info_.width, (float)scn->info_.height);
        program_base_.uniform_i("_scale", cell_scale);
        batch_.draw_batch();
    }

    if (current_paint_tex_ != nullptr) {// draw canvas
        program_canvas_.bind();
        program_canvas_.uniform_mat("_view", 4, &view[0][0]);
        program_canvas_.uniform_mat("_proj", 4, &proj[0][0]);
        current_paint_tex_->bind(0);
        program_canvas_.uniform_i("_tex", 0);
        batch_.draw_batch();
    }
    
    // detach the framebuffer texture
    glNamedFramebufferTexture(framebuf_, GL_COLOR_ATTACHMENT0, 0, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::resize_framebuffer(Dove::IVector2D _size) {
    framebuf_tex_a_.release();
    framebuf_tex_a_.init();
    framebuf_tex_a_.allocate(1, SizedInternalFormat::RGBA8, _size.x, _size.y);

    framebuf_tex_b_.release();
    framebuf_tex_b_.init();
    framebuf_tex_b_.allocate(1, SizedInternalFormat::RGBA8, _size.x, _size.y);
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
