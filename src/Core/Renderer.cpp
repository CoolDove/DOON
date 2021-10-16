#include "Renderer.h"
#include <Core/Image.h>
#include <gl/GL.h>

Renderer::Renderer(Application* _app) {
    app_ = _app;
    init_opengl();
}

void Renderer::init() {
    batch_.init({{DGL::Attribute::POSITION, 3}, { DGL::Attribute::UV, 2 }});

    shader_canvas_.load("./res/shaders/canvas.vert", "./res/shaders/canvas.frag");
    shader_base_.load("./res/shaders/base.vert", "./res/shaders/base.frag");

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
        glTextureParameteri(img_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTextureSubImage2D(img_id, 0,
                            0, 0,
                            width,
                            height,
                            GL_RGBA,
                            GL_UNSIGNED_BYTE,
                            img->pixels_);

        glBindTextureUnit(0, img_id);

        glUniform1i(glGetUniformLocation(shader_canvas_.get_id(), "_tex"), 0);

        batch_.clear();
        batch_.add_quad(width, height, "canvas");
        batch_.upload();
    }
}

void Renderer::render() {
    shader_canvas_.bind();
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    Image* img = &app_->curr_scene_->image_;
    glTextureSubImage2D(img_id, 0,
                        0, 0,
                        img->info_.width,
                        img->info_.height,
                        GL_RGBA,
                        GL_UNSIGNED_BYTE,
                        img->pixels_);

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
        int uid_view_matrix = glGetUniformLocation(shader_base_.get_id(), "_view");
        int uid_proj_matrix = glGetUniformLocation(shader_base_.get_id(), "_proj");

        int uid_size = glGetUniformLocation(shader_base_.get_id(), "_size");

        glUniformMatrix4fv(uid_view_matrix, 1, false, &view[0][0]);
        glUniformMatrix4fv(uid_proj_matrix, 1, false, &proj[0][0]);

        glUniform2f(uid_size, img->info_.width, img->info_.height);

        batch_.draw_batch();
    }
    {// draw canvas
        shader_canvas_.bind();
        int uid_view_matrix = glGetUniformLocation(shader_canvas_.get_id(), "_view");
        int uid_proj_matrix = glGetUniformLocation(shader_canvas_.get_id(), "_proj");

        int width  = app_->window_info_.width;
        int height = app_->window_info_.height;

        glUniformMatrix4fv(uid_view_matrix, 1, false, &view[0][0]);
        glUniformMatrix4fv(uid_proj_matrix, 1, false, &proj[0][0]);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
    }

}

void Renderer::on_ui() {

}