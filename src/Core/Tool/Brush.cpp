#include "Brush.h"
#include "Base/General.h"
#include "Core/Color.h"
#include "Core/Compositor.h"
#include "Core/Scene.h"
#include "DGLCore/GLEnums.h"
#include "DoveLog.hpp"
#include <Core/Application.h>
#include <Core/Space.h>
#include <DGLCore/GLDebugger.h>
#include <DGLCore/GLTexture.h>
#include <cstring>
#include <stdint.h>
#include <string.h>

// @doing: remove old img and tex pair
namespace Tool
{
Brush::Brush(Application* _app) 
:   app_(_app),
    holding_(false),
    col_{0xff,0xff,0xff,0xff},
    size_min_scale_(0.01f),
    painting_region_{0},
    size_max_(20)
{
    DLOG_TRACE("brush constructed");
}

Brush::~Brush() {
}

void Brush::on_init() {
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
        DLOG_TRACE("brush down");
        holding_ = true;
    }
}
    
void Brush::on_pointer_up(Input::PointerInfo _info, int _x, int _y) {
    using namespace Dove;
    using namespace DGL;
    if (!painting_region_.width || !painting_region_.height) return;

    if (holding_) {
        holding_ = false;
        DLOG_TRACE("brush up");

        // @Composition: composite the whole image for now
        Image* brush_img = app_->curr_scene_->brush_layer_->img_.get();
        Layer* curr_layer = app_->curr_scene_->get_curr_layer();
        IRect2D* p_region = &painting_region_;
        int width  = p_region->width;
        int height = p_region->height;
        int size_b = width * height * sizeof(Col_RGBA); // region byte size 
        BufFlag flag = BufFlag::DYNAMIC_STORAGE_BIT | BufFlag::MAP_READ_BIT | BufFlag::MAP_WRITE_BIT;

        Image src_sub(brush_img, *p_region);
        Image dst_sub(&(*curr_layer->img_), *p_region);

        uint32_t result_id = app_->compositor_->compose("common", src_sub.pixels_, dst_sub.pixels_, size_b);
        app_->compositor_->get_result(result_id, dst_sub.pixels_, size_b);
        
        // set current layer image
        curr_layer->img_->set_subimage(&dst_sub, p_region->position);
        curr_layer->mark_dirt(*p_region);
        // curr_layer->update_tex(false);

        // TODO: record brush command into commands history
        // ...
        // ...
        
        // @Clear:
        uint32_t layer_w = brush_img->info_.width;
        uint32_t layer_h = brush_img->info_.height;
        uint32_t layer_s = layer_w * layer_h * sizeof(Col_RGBA);
        memset(brush_img->pixels_, 0x00, layer_s);
        
        curr_layer->mark_dirt(*p_region);
        painting_region_ = {0};
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

        const Image* tgt_img = app_->curr_scene_->brush_layer_->img_.get();

        // draw dot on the brush img
        Dove::IRect2D dot_region =
            draw_circle((int)cs_pos.x + half_width, -(int)cs_pos.y + half_height, brush_size, tgt_img);

        painting_region_ = Dove::merge_rect(painting_region_, dot_region);
        app_->curr_scene_->brush_layer_->mark_dirt(dot_region);
    }
}

Dove::IRect2D Brush::draw_circle(int _x, int _y, int _r, const Image* _target_img) {
    if (_x < -_r || _x > _target_img->info_.width + _r || _y < -_r || _y > _target_img->info_.height + _r )
        return Dove::IRect2D{0};

    // a function changing the vec2 position into an index
    auto px = [=](int _x, int _y){
        return _y * _target_img->info_.width + _x;
    };

    int start_y = _y - _r;

    for (int i = 0; i < glm::min(2 * _r, _target_img->info_.height - start_y); i++)
    {
        int line_y = start_y + i;
        if (line_y < 0) continue;

        int scan_length = (int)(2 * glm::sqrt(_r * _r - (_r - i) * (_r - i)));
        int start_x = (int)(_x - scan_length * 0.5f);
        if (start_x < 0) {
            scan_length += start_x;
            start_x = 0;
        }

        scan_length = glm::min(scan_length, _target_img->info_.width - start_x);
        int start = px(start_x, line_y);

        for (int j = 0; j < scan_length; j++)
        {
            unsigned int* pix = (unsigned int*)_target_img->pixels_ + start + j;
            *pix = col_.cluster;
        }
    }

    // mark the updated region of current scene
    Dove::IRect2D region;
    Scene* scn = app_->curr_scene_;
    region.posx   = glm::max(_x - _r, 0);
    region.posy   = glm::max(_y - _r, 0);
    region.width  = glm::min(_x + _r, scn->info_.width) - region.posx;
    region.height = glm::min(_y + _r, scn->info_.height) - region.posy;

    return region;
}
}
