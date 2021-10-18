#include "Brush.h"
#include "DoveLog.hpp"
#include "Core/Application.h"

namespace Tool
{
Brush::Brush(Application* _app) 
:   app_(_app),
    col_{0xff,0xff,0xff,0xff},
    size_min_(10),
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

void Brush::on_pointer_down(Input::PointerInfo _info, int _x, int _y) {
    
}
void Brush::on_pointer_up(Input::PointerInfo _info, int _x, int _y) {

}

void Brush::on_pointer(Input::PointerInfo _info, int _x, int _y) {
    if (_info.btn_state.mouse_l || _info.pen_info.pressure > 1) 
    {
        int wnd_width = app_->window_info_.width;
        int wnd_height = app_->window_info_.height;

        DGL::Camera* cam = &app_->curr_scene_->camera_;
        Image* img = &app_->curr_scene_->image_;

        glm::mat4 matrix = cam->calc_proj(wnd_width, wnd_height) * cam->calc_view();
        matrix = glm::inverse(matrix);
        glm::vec4 ws_pos = glm::vec4(_x, _y, 1, 1);

        ws_pos.x = glm::clamp(ws_pos.x, 0.0f, (float)wnd_width);
        ws_pos.y = glm::clamp(ws_pos.y, 0.0f, (float)wnd_height);

        ws_pos.x = (ws_pos.x / wnd_width) * 2.0f - 1.0f;
        ws_pos.y = ((wnd_height - ws_pos.y) / wnd_height) * 2.0f - 1.0f;

        glm::vec4 cs_pos = matrix * ws_pos;

        int half_width  = (int)(0.5f * img->info_.width);
        int half_height = (int)(0.5f * img->info_.height);

        unsigned int brush_size = ((float)_info.pen_info.pressure / 1024.0f) * (size_max_ - (size_min_)) + size_min_;

        draw_circle((int)cs_pos.x + half_width, -(int)cs_pos.y + half_height, brush_size);
    }
}

void Brush::draw_circle(int _x, int _y, int _r) {
    assert(app_->curr_scene_->curr_layer_ && "no current layer selected");

    Image* img = &app_->curr_scene_->curr_layer_->img_;

    if (_x < -_r || _x > img->info_.width + _r || _y < -_r || _y > img->info_.height + _r )
        return;

    // a function changing the vec2 position into an index
    auto px = [=](int _x, int _y){
        return _y * img->info_.width + _x;
    };

    int start_y = _y - _r;

    for (int i = 0; i < glm::min(2 * _r, img->info_.height - start_y); i++)
    {
        int line_y = start_y + i;
        if (line_y < 0) 
            continue;
        
        int scan_length = (int)(2 * glm::sqrt(_r * _r - (_r - i) * (_r - i)));

        int start_x = (int)(_x - scan_length * 0.5f);
        if (start_x < 0) {
            scan_length += start_x;
            start_x = 0;
        }

        scan_length = glm::min(scan_length, img->info_.width - start_x);
        int start = px(start_x, line_y);

        for (int j = 0; j < scan_length; j++)
        {
            unsigned int* pix = (unsigned int*)img->pixels_ + start + j;
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

    // DLOG_TRACE("region:(%d, %d - %d, %d)", region.posx, region.posy, region.width, region.height);

    scn->update(region);
}
}