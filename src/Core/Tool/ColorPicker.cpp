#include "ColorPicker.h"

#include <Core/Application.h>
#include <DoveLog.hpp>
#include <Core/Scene.h>
#include <Core/Space.h>

namespace Tool
{
    ColorPicker::ColorPicker() {

    }

    ColorPicker::~ColorPicker() {
    }

    void ColorPicker::on_init() {

    }

    void ColorPicker::on_activate() {
        DLOG_DEBUG("ColorPicker activated");
        Scene* scn = Application::instance_->curr_scene_;

        GLTexture2D* tex = scn->get_composed_texture();

        size_t data_size = scn->info_.width * scn->info_.height * sizeof(Col_RGBA);
        buffer = (Col_RGBA*)malloc(data_size);

        glGetTextureImage(
            tex->get_glid(), 0,
            GL_RGBA, GL_UNSIGNED_BYTE,
            data_size, buffer);
    }

    void ColorPicker::on_deactivate() {
        DLOG_DEBUG("ColorPicker deactivated");
        free(buffer);
        buffer = nullptr;
    }

    static void worldpos_to_canvaspos(int wx, int wy, int* cx, int* cy) {
        Application* app = Application::instance_;
        int wnd_width = app->window_info_.width;
        int wnd_height = app->window_info_.height;
        DGL::Camera* cam = &app->curr_scene_->camera_;
        glm::mat4 matrix = Space::mat_ndc_world(cam, wnd_width, wnd_height);
        glm::vec4 ws_pos = glm::vec4(wx, wy, 1, 1);
        ws_pos.x = glm::clamp(ws_pos.x, 0.0f, (float)wnd_width);
        ws_pos.y = glm::clamp(ws_pos.y, 0.0f, (float)wnd_height);
        ws_pos.x = (ws_pos.x / wnd_width) * 2.0f - 1.0f;
        ws_pos.y = ((wnd_height - ws_pos.y) / wnd_height) * 2.0f - 1.0f;
        glm::vec4 cs_pos = matrix * ws_pos;
        int half_width  = (int)(0.5f * app->curr_scene_->info_.width);
        int half_height = (int)(0.5f * app->curr_scene_->info_.height);
        *cx = (int)cs_pos.x + half_width;
        *cy = -(int)cs_pos.y + half_height;
    }

    void ColorPicker::on_pointer_down(Input::PointerInfo _info, int _x, int _y) {
        int cx, cy;
        worldpos_to_canvaspos(_x, _y, &cx, &cy);
        Scene* scn = Application::instance_->curr_scene_;
        int width = scn->info_.width;
        int height = scn->info_.height;

        Col_RGBA color = buffer[cy * width + cx];
        scn->fbrush_color_.x = (float)color.r / 255.0f;
        scn->fbrush_color_.y = (float)color.g / 255.0f;
        scn->fbrush_color_.z = (float)color.b / 255.0f;
        scn->fbrush_color_.w = (float)color.a / 255.0f;
    
    }

    void ColorPicker::on_pointer_up(Input::PointerInfo _info, int _x, int _y) {
    }

    void ColorPicker::on_pointer(Input::PointerInfo _info, int _x, int _y) {
    }

    void ColorPicker::on_update() {

    }

}
