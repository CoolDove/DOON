#include "Brush.h"
#include "Base/General.h"
#include "Core/Color.h"
#include "Core/Scene.h"
#include "DoveLog.hpp"
#include <Core/Application.h>
#include <Core/Space.h>
#include <DGLCore/GLDebugger.h>
#include <DGLCore/GLTexture.h>
#include <cstring>
#include <string.h>

// @doing: remove old img and tex pair
namespace Tool
{
Brush::Brush(Application* _app) 
:   app_(_app),
    holding_(false),
    col_{0xff,0xff,0xff,0xff},
    size_min_scale_(0.01f),
    size_max_(20),
    layer_img_(_app->curr_scene_->info_.width, _app->curr_scene_->info_.height, Col_RGBA{0x00, 0x00, 0x00, 0x00}, true)
{
    DLOG_TRACE("brush constructed");

    using namespace DGL;
    try {
        Shader comp;
        comp.init(ShaderType::COMPUTE_SHADER);

        // TODO: correct the blend function
        comp.load("./res/shaders/brush-composite.comp");

        comp_shader_.link(1, &comp);
    } catch (const EXCEPTION::SHADER_COMPILING_FAILED& exp) {
        DLOG_ERROR("%s", exp.msg.c_str());
    }
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
    layer_img_.update_tex(true);
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

        using namespace DGL;
        GLTextureBuffer texbuf_src;
        GLTextureBuffer texbuf_dst;

        // @Temp: composite the whole image for now
        Layer* curr_layer = app_->curr_scene_->get_curr_layer();
        int width  = layer_img_.img_->info_.width;
        int height = layer_img_.img_->info_.height;
        int size_b = width * height * sizeof(Col_RGBA);
        BufFlag flag = BufFlag::DYNAMIC_STORAGE_BIT | BufFlag::MAP_READ_BIT | BufFlag::MAP_WRITE_BIT;

        texbuf_src.init();
        texbuf_src.allocate(size_b, flag, SizedInternalFormat::RGBA8);
        Col_RGBA* ptr_src = (Col_RGBA*)texbuf_src.buffer_->map(Access::READ_WRITE);
        memcpy(ptr_src, layer_img_.img_->pixels_, size_b);
        texbuf_src.buffer_->unmap();

        texbuf_dst.init();
        texbuf_dst.allocate(size_b, flag, SizedInternalFormat::RGBA8);
        Col_RGBA* ptr_dst = (Col_RGBA*)texbuf_dst.buffer_->map(Access::READ_WRITE);
        memcpy(ptr_dst, curr_layer->img_.pixels_, size_b);
        texbuf_dst.buffer_->unmap();
        
        texbuf_src.bind_image(0, Access::READ_WRITE, ImageUnitFormat::RGBA8UI);
        texbuf_dst.bind_image(1, Access::READ_WRITE, ImageUnitFormat::RGBA8UI);

        comp_shader_.bind();
        glDispatchCompute(width * height / 16, 1, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        ptr_dst = (Col_RGBA*)texbuf_dst.buffer_->map(Access::READ_WRITE);
        memcpy(curr_layer->img_.pixels_, ptr_dst, size_b);
        texbuf_dst.buffer_->unmap();
        // clear brush image
        memset(layer_img_.img_->pixels_, 0x00, size_b);
        // done
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

        // NOTE: we got three different target layer here:
        // - current scene layer
        // - brush layer, in an older way -- splitted tex and img
        // - brush layer, in an new way -- LayerImage

        // const Image* tgt_img = &app_->curr_scene_->get_curr_layer()->img_;
        // const Image* tgt_img = &image_;
        const Image* tgt_img = layer_img_.img_.get();// it's working!! nice
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
    Dove::IRect2D region;
    Scene* scn = app_->curr_scene_;
    region.posx   = glm::max(_x - _r, 0);
    region.posy   = glm::max(_y - _r, 0);
    region.width  = glm::min(_x + _r, scn->info_.width) - region.posx;
    region.height = glm::min(_y + _r, scn->info_.height) - region.posy;

    // scn->merge_region(region);
    scn->merge_region(region);

    // @LayerImageInte:
    layer_img_.mark_dirt(region);
}

void Brush::resize_layer_img() {
    // if (app_->curr_scene_) {
        // Scene* scn = app_->curr_scene_;
        // int width  = scn->info_.width;
        // int height = scn->info_.height;
// 
        // image_.recreate(width, height, Col_RGBA{0x00, 0x00, 0x00, 0x00});
// 
        // tex_.allocate(1, DGL::SizedInternalFormat::RGBA8, width, height);
        // tex_.upload(0, 0, 0, width, height, PixFormat::RGBA, PixType::UNSIGNED_BYTE, image_.pixels_);
    // }
}

}
