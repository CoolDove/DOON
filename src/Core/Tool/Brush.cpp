#include "Brush.h"
#include "DoveLog.hpp"
#include <Core/Application.h>
#include <Core/Space.h>
#include <DGLCore/GLDebugger.h>
#include <DGLCore/GLTexture.h>
#include <string.h>

namespace Tool
{
Brush::Brush(Application* _app) 
:   app_(_app),
    holding_(false),
    col_{0xff,0xff,0xff,0xff},
    size_min_scale_(0.01f),
    size_max_(20),
    image_(_app->curr_scene_->info_.width, _app->curr_scene_->info_.height, Col_RGBA{ 0x00, 0x00, 0x00, 0x00 })
{
    DLOG_TRACE("brush constructed");

    // TODO: load compostion shader
    using namespace DGL;

    tex_.init();
    resize_image_and_tex();
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
    // upload the whole tex for now, because it ...
    tex_.upload(0, 0, 0, image_.info_.width, image_.info_.height, PixFormat::RGBA, PixType::UNSIGNED_BYTE, image_.pixels_);
}

void Brush::on_pointer_down(Input::PointerInfo _info, int _x, int _y) {
    if (!holding_) {
        DLOG_TRACE("brush down");
        holding_ = true;
    }
}
    
void Brush::on_pointer_up(Input::PointerInfo _info, int _x, int _y) {
    if (holding_) {
        holding_ = false;
        DLOG_TRACE("brush up");
        // TODO: brush released, composite brush layer into target layer

    }
}

void Brush::on_pointer(Input::PointerInfo _info, int _x, int _y) {
    if (!holding_) return;

    if (_info.btn_state.mouse_l || _info.pen_info.pressure > 0)
    {
        float pressure = _info.pen_info.pressure ? (float)_info.pen_info.pressure : 1024.0f;

        int wnd_width = app_->window_info_.width;
        int wnd_height = app_->window_info_.height;

        DGL::Camera* cam = &app_->curr_scene_->camera_;
        Image* img = &app_->curr_scene_->image_;

        glm::mat4 matrix = Space::mat_ndc_world(cam, wnd_width, wnd_height);
        glm::vec4 ws_pos = glm::vec4(_x, _y, 1, 1);

        ws_pos.x = glm::clamp(ws_pos.x, 0.0f, (float)wnd_width);
        ws_pos.y = glm::clamp(ws_pos.y, 0.0f, (float)wnd_height);

        ws_pos.x = (ws_pos.x / wnd_width) * 2.0f - 1.0f;
        ws_pos.y = ((wnd_height - ws_pos.y) / wnd_height) * 2.0f - 1.0f;

        glm::vec4 cs_pos = matrix * ws_pos;

        int half_width  = (int)(0.5f * img->info_.width);
        int half_height = (int)(0.5f * img->info_.height);

        int size_min = (int)(size_min_scale_ * size_max_);
        unsigned int brush_size = (unsigned int)((pressure / 1024.0f) * (size_max_ - (size_min)) + size_min);

        // const Image* tgt_img = &app_->curr_scene_->get_curr_layer()->img_;
        const Image* tgt_img = &image_;

        draw_circle((int)cs_pos.x + half_width, -(int)cs_pos.y + half_height, brush_size, tgt_img);
    }
}

void Brush::draw_circle(int _x, int _y, int _r, const Image* _target_img) {
    if (_x < -_r || _x > _target_img->info_.width + _r || _y < -_r || _y > _target_img->info_.height + _r )
        return;

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
    RectInt region;
    Scene* scn = app_->curr_scene_;
    region.posx   = glm::max(_x - _r, 0);
    region.posy   = glm::max(_y - _r, 0);
    region.width  = glm::min(_x + _r, scn->info_.width) - region.posx;
    region.height = glm::min(_y + _r, scn->info_.height) - region.posy;

    scn->merge_region(region);
}

void Brush::resize_image_and_tex() {
    if (app_->curr_scene_) {
        Scene* scn = app_->curr_scene_;
        int width  = scn->info_.width;
        int height = scn->info_.height;

        image_.recreate(width, height, Col_RGBA{0x00, 0x00, 0x00, 0x00});

        tex_.allocate(1, DGL::SizedInternalFormat::RGBA8, width, height);
        tex_.upload(0, 0, 0, width, height, PixFormat::RGBA, PixType::UNSIGNED_BYTE, image_.pixels_);
    }
}

}
