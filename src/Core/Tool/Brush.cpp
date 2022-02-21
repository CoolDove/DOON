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
#include <Core/Input.h>

using namespace DGL;
namespace Tool
{
    static glm::vec2 glmvec2(Dove::IVector2D vec) {
        return { (float)vec.x, (float)vec.y };
    }
    static Dove::IVector2D dovec2(glm::vec2 vec) {
        return { (int)vec.x, (int)vec.y };
    }
    
    Brush::Brush(Application* _app)
        :   app_(_app),
    holding_(false),
    col_{0xff,0xff,0xff,0xff},
    size_min_scale_(0.01f),
    distance_(3.0),
    smooth_(1.0),
    painting_region_{0},
    size_max_(20),
    shader_(nullptr)
    {
        quad_.init({{ Attribute::POSITION, 3 }, { Attribute::UV, 2 }});
        quad_.add_quad(1, 1, "brush");
        quad_.upload();
        brush_tex_ = app_->RES->GetGLTexture2D("brush_circle_soft");
        shader_ = app_->RES->GetShader("brush_default");
    }
    
    Brush::~Brush() {
    }

    void Brush::on_init() {
    }

    Brush* Brush::ConfigMake(const SettingPair* p_settings) {
        Application* app = Application::instance_;
        Brush* brush = new Brush(app);
        for (auto ite = p_settings->cbegin(); ite != p_settings->cend(); ite++) {
            if (ite->first == "texture") {
                auto tex = app->RES->GetGLTexture2D(ite->second);
                if (tex != nullptr) {
                    brush->brush_tex_ = tex;
                } else {
                    DLOG_ERROR("failed to find texture \"%s\"", ite->second.c_str());
                    delete brush;
                    return nullptr;
                }
            } else if (ite->first == "shader") {
                auto shader = app->RES->GetShader(ite->second);
                if (shader != nullptr) {
                    brush->shader_ = shader;
                } else {
                    DLOG_ERROR("failed to find shader \"%s\"", ite->second.c_str());
                    delete brush;
                    return nullptr;
                }
            } else if (ite->first == "smooth") {
                float smooth_value;
                bool good = true;
                try {
                    size_t read = 0;
                    smooth_value = stof(ite->second, &read);
                    if (ite->second.size() != read) good = false;
                } catch (std::invalid_argument) {
                    good = false;
                }
                if (!good) {
                    DLOG_ERROR("failed to parse \"%s\" to a float", ite->second);
                    return nullptr;
                } else {
                    brush->smooth_ = smooth_value;
                }

            } else {
                DLOG_ERROR("unexpected key: %s", ite->first.c_str());
                delete brush;
                return nullptr;
            }
        }
        return brush;
    }
    
    void Brush::on_activate() {
        DLOG_TRACE("brush activated");
    }
    
    void Brush::on_deactivate() {
    }
    
    void Brush::on_update() {
    }
    
    float Brush::calculate_brush_size(const Input::PointerInfo* _info) {
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
        
        last_dap_.position = glmvec2(canvas_pos);
        last_dap_.radius = calculate_brush_size(&_info);
    }
    
    void Brush::create_blend_assets() {
        release_blend_assets();
        
        blend_framebuf_ = new GLFramebuffer();
        blend_framebuf_->init();
        
        Scene* scn = app_->curr_scene_;

        int width = scn->info_.width;
        int height = scn->info_.height;
        
        blend_tex_a_ = new GLTexture2D();
        blend_tex_a_->init();
        blend_tex_a_->allocate(1, SizedInternalFormat::RGBA8, width, height);
        
        blend_tex_b_ = new GLTexture2D();
        blend_tex_b_->init();
        blend_tex_b_->allocate(1, SizedInternalFormat::RGBA8, width, height);

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
    }
    
    void Brush::generate_daps(Dove::IVector2D mouse_pos_canvas_space, const Input::PointerInfo* brush_info) {
        daps_.clear();

        auto get_daprect = [](glm::vec2 pos, uint32_t brush_radius) -> Dove::IRect2D {
            using namespace Dove;
            Dove::IVector2D dvecpos = dovec2(pos);
            IRect2D rect;
            uint32_t rect_size = (uint32_t)(1.425f * (float)brush_radius) + 1;
            if (rect_size % 2 == 1) rect_size += 1;
            rect.position = { dvecpos.x - (int)rect_size / 2, dvecpos.y - (int)rect_size / 2 };
            rect.size = { rect_size, rect_size };
            return rect;
        };

        glm::vec2 mpos = glmvec2(mouse_pos_canvas_space);// current target
        float dist = glm::clamp(distance_, 0.5f, distance_);

        float brush_size = calculate_brush_size(brush_info);

        #define DRAW_MULTI_DAPS
        #ifdef DRAW_MULTI_DAPS
            const static int MAX_STEP = 32;
            int step = 0;
            while (step < MAX_STEP) {
                // glm::vec2 last_pos_glm = glmvec2(last_dap_pos_);
                float stepdistance = glm::distance(last_dap_.position, mpos);
                if (stepdistance < dist) {
                    break;
                }
                glm::vec2 direction = glm::normalize(mpos - last_dap_.position);
                last_dap_.position += direction * dist;
                last_dap_.radius = brush_size;

                daps_.push_back(last_dap_);
                painting_region_ = Dove::merge_rect(painting_region_, get_daprect(last_dap_.position, brush_size));
                step++;
            }
        #else
            BrushDap dap = {
                mouse_pos_canvas_space,
                0,
                brush_size
            };
            daps_.push_back(dap);
            auto rect = get_daprect(dap.position, brush_size);
            painting_region_ = Dove::merge_rect(painting_region_, rect);
        #endif
    }
    
    void Brush::draw_daps() {
        if (daps_.size() == 0) return;
        using namespace DGL;
        
        auto* shader = shader_;

        if (!shader) {
            DLOG_ERROR("failed to find brush shader");
            return;
        }

        Scene* scn = app_->curr_scene_;
        
        Dove::IRect2D blitrect;
        blitrect.position = {0, 0};
        blitrect.size = {(uint32_t)scn->info_.width, (uint32_t)scn->info_.height};
        Renderer::blit(&scn->brush_layer_, blend_other_tex(), painting_region_, painting_region_);
        
        shader->bind();
        blend_framebuf_->bind();
        blend_framebuf_->attach(&scn->brush_layer_);
        glViewport(0, 0, scn->info_.width, scn->info_.height);
        
        glDisable(GL_BLEND);

        blend_other_tex()->bind(0);
        shader->uniform_i("_paintbuffer", 0);
        brush_tex_->bind(1);
        shader->uniform_i("_brushtex", 1);

        shader->uniform_f(
            "_brushcol", (float)col_.r/255.0, (float)col_.g/255.0, (float)col_.b/255.0, (float)col_.a/255.0);

        shader->uniform_f("_canvassize", (float)scn->info_.width, (float)scn->info_.height);
        
        for (auto ite = daps_.begin(); ite != daps_.end(); ite++) {
            shader->uniform_f("_dappos", (float)ite->position.x, (float)ite->position.y);
            shader->uniform_f("_dapsize", (float)ite->radius);
            quad_.draw_batch();
            Renderer::blit(&scn->brush_layer_, blend_other_tex(), painting_region_, painting_region_);
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
        
        clear_brush_tex({0x00, 0x00, 0x00, 0x00});
    }
    
    void Brush::on_pointer_up(Input::PointerInfo _info, int _x, int _y) {
        using namespace Dove;
        using namespace DGL;
        if (!painting_region_.width || !painting_region_.height) return;
        
        if (holding_) {
            holding_ = false;
            
            flush_data();
            
            release_blend_assets();
            last_dap_.position = { -1.0f, -1.0f };
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
            Scene* scn = app_->curr_scene_;
            float pressure = _info.pen_info.pressure ? (float)_info.pen_info.pressure : 1024.0f;
            
            unsigned int brush_size = calculate_brush_size(&_info);

            Dove::IVector2D canvas_pos;
            worldpos_to_canvaspos(_x, _y, (int*)&canvas_pos.x, (int*)&canvas_pos.y);
            
            generate_daps(canvas_pos, &_info);
            draw_daps();
        }
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
