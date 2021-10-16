#include "Brush.h"
#include "DoveLog.hpp"
#include "Core/Application.h"

namespace Tool
{
Brush::Brush(Application* _app) 
:   app_(_app),
    col_{1,1,1,1}
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
        // DLOG_TRACE("dragging mouse");
        // transform mouseposition to image space from window space
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
        
        unsigned int brush_size = ((float)_info.pen_info.pressure / 1024.0f) * 20 + 5;
        unsigned int cr, cg, cb, ca = 0;
        cr = (unsigned int)(col_[0] * 255);
        cg = (unsigned int)(col_[1] * 255);
        cb = (unsigned int)(col_[2] * 255);
        ca = (unsigned int)(col_[3] * 255);

        unsigned int ucol = 0;
        ucol |= (cr << 24);
        ucol |= (cg << 16);
        ucol |= (cb <<  8);
        ucol |= (ca <<  0);

        draw_circle((int)cs_pos.x + half_width, -(int)cs_pos.y + half_height, brush_size, ucol);

    }
}

void Brush::draw_circle(int _x, int _y, int _r, unsigned int _col) {
    Image* img = &app_->curr_scene_->image_;

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

        scan_length = glm::min(scan_length, img->info_.width - start_x - 1);
        int start = px(start_x, line_y);

        for (int j = 0; j < scan_length; j++)
        {
            char* col = (char*)&_col;
            char* pix = (char*)((int*)img->pixels_ + start + j);
            
            int check = 0x00ffffff;

            if (*((char*)&check) == 0x00) {
                pix[0] = col[0];
                pix[1] = col[1];
                pix[2] = col[2];
                pix[3] = col[3];
            } else {
                pix[0] = col[3];
                pix[1] = col[2];
                pix[2] = col[1];
                pix[3] = col[0];
            }
        }
    }
}

}