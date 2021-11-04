#include "Brush.h"
#include "Base/General.h"
#include "Core/Color.h"
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
    size_max_(20),
    brush_layer_img_(_app->curr_scene_->info_.width, _app->curr_scene_->info_.height, Col_RGBA{0x00, 0x00, 0x00, 0x00}, true)
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
    brush_layer_img_.update_tex(false);
}

void Brush::on_pointer_down(Input::PointerInfo _info, int _x, int _y) {
    if (!holding_) {
        DLOG_TRACE("brush down");
        holding_ = true;
    }
}
    
void Brush::on_pointer_up(Input::PointerInfo _info, int _x, int _y) {
    if (!painting_region_.width || !painting_region_.height) return;

    if (holding_) {
        holding_ = false;
        DLOG_TRACE("brush up");

        using namespace DGL;
        GLTextureBuffer texbuf_src;
        GLTextureBuffer texbuf_dst;

        // @Composition: composite the whole image for now
        Layer* curr_layer = app_->curr_scene_->get_curr_layer();
        Dove::IRect2D* p_region = &painting_region_;
        int width  = p_region->width;
        int height = p_region->height;
        int size_b = width * height * sizeof(Col_RGBA); // region byte size 
        BufFlag flag = BufFlag::DYNAMIC_STORAGE_BIT | BufFlag::MAP_READ_BIT | BufFlag::MAP_WRITE_BIT;

        // NOTE: subimage that stores the data to be composed
        Image src_sub(brush_layer_img_.img_.get(), *p_region);
        Image dst_sub(&curr_layer->img_, *p_region);

        texbuf_dst.init();
        texbuf_dst.allocate(size_b, flag, SizedInternalFormat::RGBA8);

        texbuf_src.init();
        texbuf_src.allocate(size_b, flag, SizedInternalFormat::RGBA8);

        Col_RGBA* ptr_src = (Col_RGBA*)texbuf_src.buffer_->map(Access::READ_WRITE);
        Col_RGBA* ptr_dst = (Col_RGBA*)texbuf_dst.buffer_->map(Access::READ_WRITE);

        Col_RGBA* ptr_src_img_ = src_sub.pixels_;
        memcpy(ptr_src, ptr_src_img_, size_b);
        Col_RGBA* ptr_dst_img_ = dst_sub.pixels_;
        memcpy(ptr_dst, ptr_dst_img_, size_b);

        texbuf_src.buffer_->unmap();
        texbuf_dst.buffer_->unmap();
        
        texbuf_src.bind_image(0, Access::READ_WRITE, ImageUnitFormat::RGBA8UI);
        texbuf_dst.bind_image(1, Access::READ_WRITE, ImageUnitFormat::RGBA8UI);

        comp_shader_.bind();
        glDispatchCompute(width * height / 16, 1, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // @ApplyResult:
        Col_RGBA* result = (Col_RGBA*)texbuf_dst.buffer_->map(Access::READ_WRITE);
        memcpy(dst_sub.pixels_, result, size_b);
        curr_layer->img_.set_subimage(&dst_sub, p_region->position);
        texbuf_dst.buffer_->unmap();


        // TODO: record brush command into commands history
        // ...
        
        // @Clear:
        // clear brush layer, then mark the whole brush layer image as dirty,
        // so that next rendering it will be blank
        uint32_t layer_w = brush_layer_img_.img_->info_.width;
        uint32_t layer_h = brush_layer_img_.img_->info_.height;
        uint32_t layer_s = layer_w * layer_h * sizeof(Col_RGBA);
        memset(brush_layer_img_.img_->pixels_, 0x00, layer_s);
        brush_layer_img_.mark_dirt(*p_region);
        // mark the scene layer img to be dirty, let it recompose all the needed layers
        app_->curr_scene_->merge_region(*p_region);

        // clear painting_region
        painting_region_ = {0};
        // done
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

        const Image* tgt_img = brush_layer_img_.img_.get();
        Dove::IRect2D step_region = draw_circle((int)cs_pos.x + half_width, -(int)cs_pos.y + half_height, brush_size, tgt_img);

        painting_region_ = Dove::merge_rect(painting_region_, step_region);
        brush_layer_img_.mark_dirt(step_region);

        app_->curr_scene_->merge_region(step_region);
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
