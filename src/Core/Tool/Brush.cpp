#include "Brush.h"
#include <Core/Command_Brush.h>

#include <Base/General.h>
#include <Core/Color.h>
#include <Core/Scene.h>
#include <DGLCore/GLEnums.h>
#include <DGLCore/GLGeoBatch.h>
#include <DoveLog.hpp>
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
#include <Core/History.h>

// @doing: remove old img and tex pair
using namespace DGL;
namespace Tool
{
Brush::Brush(Application* _app) 
:   app_(_app),
    holding_(false),
    col_{0xff,0xff,0xff,0xff},
    size_min_scale_(0.01f),
    distance_(6.0),
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

float Brush::calculate_brush_size(Input::PointerInfo* _info) {
    float pressure = _info->pen_info.pressure ? (float)_info->pen_info.pressure : 1024.0f;

    int size_min = (int)(size_min_scale_ * size_max_);

    unsigned int brush_size = (unsigned int)((pressure / 1024.0f) * (size_max_ - (size_min)) + size_min);
    return brush_size;
}

void Brush::on_pointer_down(Input::PointerInfo _info, int _x, int _y) {
    if (holding_) return;
    holding_ = true;

    create_blend_assets();

    Dove::IVector2D canvas_pos;
    worldpos_to_canvaspos(_x, _y, (int*)&canvas_pos.x, (int*)&canvas_pos.y);

    last_dap_pos_ = canvas_pos;
    last_mouse_pos_ = canvas_pos;
    last_brush_size_ = calculate_brush_size(&_info);
}

void Brush::create_blend_assets() {
    release_blend_assets();

    blend_framebuf_ = new GLFramebuffer();
    blend_framebuf_->init();

    Scene* scn = app_->curr_scene_;

    blend_tex_a_ = new GLTexture2D();
    blend_tex_a_->init();
    blend_tex_a_->allocate(1, SizedInternalFormat::RGBA8, scn->info_.width, scn->info_.height);

    blend_tex_b_ = new GLTexture2D();
    blend_tex_b_->init();
    blend_tex_b_->allocate(1, SizedInternalFormat::RGBA8, scn->info_.width, scn->info_.height);

    switch_attaching_texture();
}

void Brush::release_blend_assets() {
    if (blend_tex_a_ != nullptr) delete blend_tex_a_;
    if (blend_tex_b_ != nullptr) delete blend_tex_b_;
    if (blend_framebuf_ != nullptr) delete blend_framebuf_;
    blend_tex_a_ = nullptr;
    blend_tex_b_ = nullptr;
    blend_framebuf_ = nullptr;
    blend_attaching_ = nullptr;
}

void Brush::switch_attaching_texture() {
    if (blend_attaching_ == nullptr) blend_attaching_ = blend_tex_a_;
    else if (blend_attaching_ == blend_tex_a_) blend_attaching_ = blend_tex_b_;
    else if (blend_attaching_ == blend_tex_b_) blend_attaching_ = blend_tex_a_;

    if (blend_attaching_ != nullptr && blend_framebuf_ != nullptr) {
        blend_framebuf_->attach(blend_attaching_);
    }
}

static glm::vec2 glmvec2(Dove::IVector2D vec) {
    return { (float)vec.x, (float)vec.y };
}
static Dove::IVector2D dovec2(glm::vec2 vec) {
    return { (int)vec.x, (int)vec.y };
}

void Brush::generate_daps(Dove::IVector2D mouse_pos_canvas_space, float brush_size) {
    daps_.clear();
    glm::vec2 start_pos = glmvec2(last_dap_pos_);
    glm::vec2 mpos = glmvec2(mouse_pos_canvas_space);
    glm::vec2 direction = glm::normalize(mpos - glmvec2(last_dap_pos_));
    float target_dist = glm::distance(glmvec2(last_dap_pos_), mpos);
    float dist = glm::clamp(distance_, 1.0f, distance_);
    for (float i = 0; glm::distance(start_pos, glmvec2(last_dap_pos_)) < target_dist; i += dist) {
        last_dap_pos_ = dovec2(glmvec2(last_dap_pos_) + direction * dist);
        BrushDap dap = {
            last_dap_pos_,
            0,
            brush_size
        };
        daps_.push_back(dap);
    }
}

void Brush::draw_daps() {
    if (daps_.size() == 0) return;

    auto* shader = app_->RES->GetShader("dap");
    if (!shader) {
        DLOG_ERROR("failed to find shader: dap");
        return;
    }
    Scene* scn = app_->curr_scene_;

    shader->bind();
    blend_framebuf_->bind();
    glViewport(0, 0, scn->info_.width, scn->info_.height);
    glDisable(GL_BLEND);

    shader->uniform_i("_paintbuffer", 0);
    brush_tex_->bind(1);
    shader->uniform_i("_brushtex", 1);

    for (auto ite = daps_.begin(); ite != daps_.end(); ite++) {

        if (ite == --daps_.end())
            blend_framebuf_->attach(&scn->brush_layer_);

        // TODO: draw the dap to the frame buffer, use blend_other_tex() to blend
        blend_other_tex()->bind(0);

        shader->uniform_f("_dappos", (float)ite->position.x, (float)ite->position.y);
        shader->uniform_f("_canvassize", (float)scn->info_.width, (float)scn->info_.height);
        shader->uniform_f("_dapsize", (float)ite->radius);

        quad_.draw_batch();

        switch_attaching_texture();
    }
}

void Brush::flush_data() {
    using namespace Dove;
    BrushCommand* cmd = new BrushCommand(painting_region_, app_);
    app_->curr_scene_->get_history_sys()->push(cmd);

    // @Composition: composite the whole image for now
    glDisable(GL_BLEND);
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

    painting_region_ = {0};

    clear_brush_tex({0xff, 0xff, 0xff, 0x00});
}
    
void Brush::on_pointer_up(Input::PointerInfo _info, int _x, int _y) {
    using namespace Dove;
    using namespace DGL;
    if (!painting_region_.width || !painting_region_.height) return;

    if (holding_) {
        holding_ = false;

        flush_data();

        release_blend_assets();
        last_dap_pos_ = { -1, -1 };
    }
}

void Brush::worldpos_to_canvaspos(int wx, int wy, int* cx, int* cy) {
    int wnd_width = app_->window_info_.width;
    int wnd_height = app_->window_info_.height;
    DGL::Camera* cam = &app_->curr_scene_->camera_;
    glm::mat4 matrix = Space::mat_ndc_world(cam, wnd_width, wnd_height);
    glm::vec4 ws_pos = glm::vec4(wx, wy, 1, 1);
    ws_pos.x = glm::clamp(ws_pos.x, 0.0f, (float)wnd_width);
    ws_pos.y = glm::clamp(ws_pos.y, 0.0f, (float)wnd_height);
    ws_pos.x = (ws_pos.x / wnd_width) * 2.0f - 1.0f;
    ws_pos.y = ((wnd_height - ws_pos.y) / wnd_height) * 2.0f - 1.0f;
    glm::vec4 cs_pos = matrix * ws_pos;
    int half_width  = (int)(0.5f * app_->curr_scene_->info_.width);
    int half_height = (int)(0.5f * app_->curr_scene_->info_.height);
    *cx = (int)cs_pos.x + half_width;
    *cy = -(int)cs_pos.y + half_height;
}

void Brush::on_pointer(Input::PointerInfo _info, int _x, int _y) {
    if (!holding_) return;

    if (_info.btn_state.mouse_l || _info.pen_info.pressure > 0)
    {
        // TODO: paint multiple brush ink dots
        Scene* scn = app_->curr_scene_;
        float pressure = _info.pen_info.pressure ? (float)_info.pen_info.pressure : 1024.0f;

        unsigned int brush_size = calculate_brush_size(&_info);

        Dove::IVector2D canvas_pos;
        worldpos_to_canvaspos(_x, _y, (int*)&canvas_pos.x, (int*)&canvas_pos.y);

        generate_daps(canvas_pos, brush_size);

        draw_daps();

        // NOTE: build brush region for this stroke
        Dove::IRect2D rect;
        float extend_size = glm::floor(1.42f * glm::max((float)brush_size, last_brush_size_));
        rect.posx = glm::min(last_mouse_pos_.x, canvas_pos.x);
        rect.posy = glm::min(last_mouse_pos_.y, canvas_pos.y);
        rect.width = glm::max(last_mouse_pos_.x, canvas_pos.x) - rect.posx;
        rect.height = glm::max(last_mouse_pos_.y, canvas_pos.y) - rect.posy;
        rect.posx -= rect.width * 0.5f;
        rect.posx = glm::clamp(rect.posx, 0, scn->info_.width);
        rect.posy -= rect.height * 0.5f;
        rect.posy = glm::clamp(rect.posy, 0, scn->info_.height);
        rect.width += extend_size;
        rect.width = glm::clamp(rect.width, 0u, (uint32_t)(scn->info_.width - rect.posx));
        rect.height += extend_size;
        rect.height = glm::clamp(rect.height, 0u, (uint32_t)(scn->info_.height - rect.posy));

        painting_region_ = Dove::merge_rect(painting_region_, rect);

        last_brush_size_ = calculate_brush_size(&_info);
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
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        glBindFramebuffer(GL_FRAMEBUFFER, fbuf_brush_);
        static const GLenum draw_buffers[] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, draw_buffers);
        glViewport(0, 0, scn->info_.width, scn->info_.height);

        // auto texid = scn->get_curr_layer()->tex_->get_glid(); // draw to current layer directly
        auto texid = scn->brush_layer_.get_glid();
        glNamedFramebufferTexture(fbuf_brush_, GL_COLOR_ATTACHMENT0, texid, 0);

        shader_->bind();
        brush_tex_->bind(0);
        shader_->uniform_i("_brushtex", 0);
        auto fcol = get_float_col(col_);
        shader_->uniform_f("_brushcol", fcol.r, fcol.g, fcol.b, fcol.a);
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

    auto fcol = get_float_col(color);
    glClearColor(fcol.r, fcol.g, fcol.b, fcol.a);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glBindFramebuffer(GL_FRAMEBUFFER, fbuf_stash);

    painting_region_ = {0};
}

}
