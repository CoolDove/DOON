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
Renderer::Renderer(Application* _app) {
    app_ = _app;
    init_opengl();
}

void Renderer::init() {
    batch_.init({{DGL::Attribute::POSITION, 3}, { DGL::Attribute::UV, 2 }});

    std::string msg_canvas_vert;
    std::string msg_canvas_frag;

    try {
        Shader canvas_vert("./res/shaders/canvas.vert", ShaderType::VERTEX_SHADER, &msg_canvas_vert);
        Shader canvas_frag("./res/shaders/canvas.frag", ShaderType::FRAGMENT_SHADER, &msg_canvas_frag);
        program_canvas_.link(2, &canvas_vert, &canvas_frag);
    } catch (const DGL::EXCEPTION::SHADER_COMPILING_FAILED& err) {
        DLOG_ERROR("shader error: %s", err.msg.c_str());
    }

    Shader base_vert("./res/shaders/base.vert", ShaderType::VERTEX_SHADER);
    Shader base_frag("./res/shaders/base.frag", ShaderType::FRAGMENT_SHADER);
    program_base_.link(2, &base_vert, &base_frag);

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

void Renderer::render() {
    program_canvas_.bind();
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    Scene* scn = app_->curr_scene_;
    Dove::IRect2D updated_region = scn->get_region();

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int wnd_width  = app_->window_info_.width;
    int wnd_height = app_->window_info_.height;

    DGL::Camera* cam = &app_->curr_scene_->camera_;
    glm::mat4 view = Space::mat_world_camera(&app_->curr_scene_->camera_);
    glm::mat4 proj = Space::mat_camproj(&app_->curr_scene_->camera_, wnd_width, wnd_height);

    {// @DrawBase:
        program_base_.bind();
        float cam_size = (10.0f - cam->size_)/10.0f;
        int cell_scale = (int)((cam_size * cam_size * cam_size) * 30 + 1);
        program_base_.uniform_mat("_view", 4, &view[0][0]);
        program_base_.uniform_mat("_proj", 4, &proj[0][0]);
        program_base_.uniform_f("_size", (float)scn->info_.width, (float)scn->info_.height);
        program_base_.uniform_i("_scale", cell_scale);
        batch_.draw_batch();
    }

    {// @DrawLayers:
        for (auto ite = scn->layers_.begin(); ite != scn->layers_.end(); ite++) {
            program_canvas_.bind();
            program_canvas_.uniform_mat("_view", 4, &view[0][0]);
            program_canvas_.uniform_mat("_proj", 4, &proj[0][0]);

            program_canvas_.uniform_i("_tex", 0);
            ite->get()->tex_->bind(0);

            batch_.draw_batch();
            if (ite->get() == scn->get_curr_layer()) {
                program_canvas_.uniform_i("_tex", 0);
                scn->brush_tex_.bind(0);

                batch_.draw_batch();
            }
        }
    }
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

    glEnable(GL_BLEND);
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
