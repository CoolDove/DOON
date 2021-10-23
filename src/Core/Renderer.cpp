﻿#include "Renderer.h"
#include <Core/Image.h>
#include <gl/GL.h>
#include "DoveLog.hpp"
#include <DGLCore/GLDebugger.h>
#include <DGLCore/GLShader.h>

using namespace DGL;

Renderer::Renderer(Application* _app) {
    app_ = _app;
    init_opengl();
}

void Renderer::init() {
    batch_.init({{DGL::Attribute::POSITION, 3}, { DGL::Attribute::UV, 2 }});

    std::string msg_canvas_vert;
    std::string msg_canvas_frag;

    Shader canvas_vert("./res/shaders/canvas.vert", ShaderType::VERTEX_SHADER, &msg_canvas_vert);
    Shader canvas_frag("./res/shaders/canvas.frag", ShaderType::FRAGMENT_SHADER, &msg_canvas_frag);
    shader_canvas_.link({ &canvas_vert, &canvas_frag });

    Shader base_vert("./res/shaders/base.vert", ShaderType::VERTEX_SHADER);
    Shader base_frag("./res/shaders/base.frag", ShaderType::FRAGMENT_SHADER);
    shader_base_.link({ &base_vert, &base_frag });

    create_gl_image();
}

void Renderer::create_gl_image() {
    if (app_->curr_scene_ && app_->curr_scene_->image_.pixels_) {
        if (glIsTexture(img_id)) {
            glDeleteTextures(1, &img_id);
        }

        Image* img = &app_->curr_scene_->image_;
        int width  = app_->curr_scene_->image_.info_.width;
        int height = app_->curr_scene_->image_.info_.height;

        glCreateTextures(GL_TEXTURE_2D, 1, &img_id);
        glTextureStorage2D(img_id, 1, GL_RGBA8, width, height);

        glTextureParameteri(img_id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTextureParameteri(img_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(img_id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(img_id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        glTextureSubImage2D(img_id, 0,
                            0, 0,
                            width,
                            height,
                            GL_RGBA,
                            GL_UNSIGNED_BYTE,
                            img->pixels_);

        batch_.clear();
        batch_.add_quad((float)width, (float)height, "canvas");
        batch_.upload();
    }
}

void Renderer::render() {
    shader_canvas_.bind();
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    Scene* scn = app_->curr_scene_;
    Image* img = &app_->curr_scene_->image_;

    if (scn->region_.width != 0 && scn->region_.height != 0) {
        // upload the modified region row by row
        // test
        // long time = std::clock();
        // glTextureSubImage2D(img_id, 0,
        //                     scn->region_.posx, 
        //                     scn->region_.posy, 
        //                     scn->region_.width, 
        //                     scn->region_.height, 
        //                     GL_RGBA,
        //                     GL_UNSIGNED_BYTE,
        //                     img->pixels_);
        // DLOG_TRACE("A %ld ms", std::clock() - time);
        // test

        // time = std::clock();

        for (int i = scn->region_.posy; i < scn->region_.posy + scn->region_.height; i++) {
            glTextureSubImage2D(img_id, 0,
                                scn->region_.posx, 
                                i,
                                scn->region_.width, 
                                1,
                                GL_RGBA,
                                GL_UNSIGNED_BYTE,
                                img->pixels_ + i * 4 * scn->info_.width + scn->region_.posx * 4);
        }
        memset(&scn->region_, 0, sizeof(RectInt));
    };

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    int width  = app_->window_info_.width;
    int height = app_->window_info_.height;

    DGL::Camera* cam = &app_->curr_scene_->camera_;
    glm::mat4 view = cam->calc_view();
    glm::mat4 proj = cam->calc_proj(width, height);

    {// draw base
        shader_base_.bind();
        int uid_view_matrix = glGetUniformLocation(shader_base_.get_glid(), "_view");
        int uid_proj_matrix = glGetUniformLocation(shader_base_.get_glid(), "_proj");

        int uid_size  = glGetUniformLocation(shader_base_.get_glid(), "_size");
        int uid_scale = glGetUniformLocation(shader_base_.get_glid(), "_scale");

        glUniformMatrix4fv(uid_view_matrix, 1, false, &view[0][0]);
        glUniformMatrix4fv(uid_proj_matrix, 1, false, &proj[0][0]);

        glUniform2f(uid_size, (float)img->info_.width, (float)img->info_.height);

        float cam_size = (10.0f - cam->size_)/10.0f;
        int cell_scale = (int)((cam_size * cam_size * cam_size) * 30 + 1);

        glUniform1i(uid_scale, cell_scale);

        batch_.draw_batch();
    }
    {// draw canvas
        shader_canvas_.bind();
        int uid_view_matrix = glGetUniformLocation(shader_canvas_.get_glid(), "_view");
        int uid_proj_matrix = glGetUniformLocation(shader_canvas_.get_glid(), "_proj");

        int width  = app_->window_info_.width;
        int height = app_->window_info_.height;

        glUniformMatrix4fv(uid_view_matrix, 1, false, &view[0][0]);
        glUniformMatrix4fv(uid_proj_matrix, 1, false, &proj[0][0]);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindTextureUnit(0, img_id);
        // glBindTextureUnit(0, app_->buf_tex_);
        glUniform1i(glGetUniformLocation(shader_canvas_.get_glid(), "_tex"), 0);

        batch_.draw_batch();
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