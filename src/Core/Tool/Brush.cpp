#include "Brush.h"
#include "Base/General.h"
#include "Core/Color.h"
#include "Core/Scene.h"
#include "DGLCore/GLEnums.h"
#include "DGLCore/GLGeoBatch.h"
#include "DoveLog.hpp"
#include <Core/Application.h>
#include <Core/Space.h>
#include <DGLCore/GLDebugger.h>
#include <DGLCore/GLTexture.h>
#include <DGLCore/GLFramebuffer.h>
#include <cstring>
#include <stdint.h>
#include <string.h>
#include <Core/DOONRes.h>
#include <Core/Renderer.h>

// @doing: remove old img and tex pair
using namespace DGL;
namespace Tool
{
Brush::Brush(Application* _app) 
:   app_(_app),
    holding_(false),
    col_{0xff,0xff,0xff,0xff},
    size_min_scale_(0.01f),
    painting_region_{0},
    size_max_(20),
    shader_(nullptr)
{
    DLOG_TRACE("brush constructed");
}

Brush::~Brush() {
}
void Brush::on_init() {
    brush_tex_ = app_->RES->LoadGLTexture2D("./res/brushes/ugly.png", "brush_ugly");
    DLOG_DEBUG("brush texture loaded: ugly");
    if (shader_ = app_->RES->LoadShader("./res/shaders/brush.vert", "./res/shaders/brush.frag", "brush")) {
        DLOG_DEBUG("brush shader loaded");
    } else {
        DLOG_ERROR("failed to load brush shader");
    }

    quad_.init({{ Attribute::POSITION, 3 }, { Attribute::UV, 2 }});
    quad_.add_quad(1, 1, "brush");
    quad_.upload();

    glCreateFramebuffers(1, &fbuf_brush_);
}

void Brush::on_activate() {
    DLOG_TRACE("brush activated");
}

void Brush::on_deactivate() {
}

void Brush::on_update() {
}

void Brush::on_pointer_down(Input::PointerInfo _info, int _x, int _y) {
    if (!holding_) {
        holding_ = true;
    }
}
    
void Brush::on_pointer_up(Input::PointerInfo _info, int _x, int _y) {
    using namespace Dove;
    using namespace DGL;
    if (!painting_region_.width || !painting_region_.height) return;

    if (holding_) {
        holding_ = false;
        // @Composition: composite the whole image for now
        GLTexture2D* brush_texture = &(app_->curr_scene_->brush_layer_);
        Layer* curr_layer = app_->curr_scene_->get_curr_layer();

        GLTexture2D* src = brush_texture;
        GLTexture2D* dst = app_->curr_scene_->get_curr_layer()->tex_.get();
        GLTexture2D temp;
        temp.init();
        temp.allocate(1, SizedInternalFormat::RGBA8, src->info_.width, src->info_.height, PixFormat::RGBA, PixType::UNSIGNED_BYTE, nullptr);
        
        GLFramebuffer fbuf;
        fbuf.init();
        fbuf.bind();
        fbuf.attach(&temp);

        glViewport(0, 0, dst->info_.width, dst->info_.height);
        glDepthFunc(GL_ALWAYS);

        auto paint_shader = app_->RES->GetShader("paint");

        paint_shader->bind();
        paint_shader->uniform_f("_size", (float)src->info_.width, (float)src->info_.height);
        src->bind(0);
        paint_shader->uniform_i("_tex", 0);
        dst->bind(1);
        paint_shader->uniform_i("_paintbuffer", 1);

        app_->renderer_->get_canvas_quad()->draw_batch();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        IRect2D rect;
        rect.position = {0, 0};
        rect.size = {src->info_.width, src->info_.height};
        Renderer::blit(&temp, dst, rect, rect);

        curr_layer->mark_dirt(painting_region_);

        painting_region_ = {0};
        clear_brush_tex();

    }
}

void Brush::on_pointer(Input::PointerInfo _info, int _x, int _y) {
    if (!holding_) return;

    if (_info.btn_state.mouse_l || _info.pen_info.pressure > 0)
    {
        // TODO: paint multiple brush ink dots
        float pressure = _info.pen_info.pressure ? (float)_info.pen_info.pressure : 1024.0f;

        int wnd_width = app_->window_info_.width;
        int wnd_height = app_->window_info_.height;

        DGL::Camera* cam = &app_->curr_scene_->camera_;

        glm::mat4 matrix = Space::mat_ndc_world(cam, wnd_width, wnd_height);
        glm::vec4 ws_pos = glm::vec4(_x, _y, 1, 1);

        ws_pos.x = glm::clamp(ws_pos.x, 0.0f, (float)wnd_width);
        ws_pos.y = glm::clamp(ws_pos.y, 0.0f, (float)wnd_height);

        ws_pos.x = (ws_pos.x / wnd_width) * 2.0f - 1.0f;
        ws_pos.y = ((wnd_height - ws_pos.y) / wnd_height) * 2.0f - 1.0f;

        glm::vec4 cs_pos = matrix * ws_pos;

        int half_width  = (int)(0.5f * app_->curr_scene_->info_.width);
        int half_height = (int)(0.5f * app_->curr_scene_->info_.height);

        int size_min = (int)(size_min_scale_ * size_max_);
        unsigned int brush_size = (unsigned int)((pressure / 1024.0f) * (size_max_ - (size_min)) + size_min);

        // const Image* tgt_img = app_->curr_scene_->brush_layer_->img_.get();

        // draw dot on the brush img
        Dove::IRect2D dot_region = draw_circle(
            (int)cs_pos.x + half_width, -(int)cs_pos.y + half_height, brush_size);

        painting_region_ = Dove::merge_rect(painting_region_, dot_region);
    }
}

Dove::IRect2D Brush::draw_circle(int _x, int _y, int _r) {
    Scene* scn = app_->curr_scene_;
    if (_x < -_r || _x > scn->info_.width + _r || _y < -_r || _y > scn->info_.height + _r)
        return Dove::IRect2D{0};

    if (shader_ == nullptr || brush_tex_ == nullptr) return {0};

    // DLOG_DEBUG("pos: %d, %d", _x, _y);
    {// Draw to current brush layer
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindFramebuffer(GL_FRAMEBUFFER, fbuf_brush_);
        static const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, draw_buffers);
        glViewport(0, 0, scn->info_.width, scn->info_.height);

        auto texid = scn->brush_layer_.get_glid(); // brush layer texture
        glNamedFramebufferTexture(fbuf_brush_, GL_COLOR_ATTACHMENT0, texid, 0);

        shader_->bind();
        brush_tex_->bind(0);
        shader_->uniform_i("_brushtex", 0);
        shader_->uniform_f("_dappos", (float)_x, (float)_y);
        shader_->uniform_f("_canvassize", (float)scn->info_.width, (float)scn->info_.height);
        shader_->uniform_f("_dapsize", (float)_r);

        quad_.draw_batch();

        glNamedFramebufferTexture(fbuf_brush_, GL_COLOR_ATTACHMENT0, 0, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // mark the updated region of current scene
    Dove::IRect2D region;
    region.posx   = glm::max(_x - _r, 0);
    region.posy   = glm::max(_y - _r, 0);
    region.width  = glm::min(_x + _r, scn->info_.width) - region.posx;
    region.height = glm::min(_y + _r, scn->info_.height) - region.posy;

    return region;
}

void Brush::clear_brush_tex(Col_RGBA color) {
    GLint fbuf_stash = GLFramebuffer::current_framebuffer();

    GLFramebuffer fbuf;

    fbuf.init();
    fbuf.bind();

    fbuf.attach(&app_->curr_scene_->brush_layer_);

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glBindFramebuffer(GL_FRAMEBUFFER, fbuf_stash);

    painting_region_ = {0};
}

}
