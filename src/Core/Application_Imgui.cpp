#include "Application.h"
#include <imgui/imgui.h>

void Application::gui_BrushChooser() {
    if (ImGui::CollapsingHeader("brushes")) {
        for (auto ite = brushes_.begin(); ite != brushes_.end(); ite++) {
            if (curr_tool_ == ite->second) {
                ImGui::Bullet();
                ImGui::SameLine();
            }
            if (ImGui::Selectable(ite->first.c_str())) {
                curr_tool_ = ite->second;
            }
        }
    }
}


void Application::gui_ColorPicker() {
    if (ImGui::CollapsingHeader("Color")) {
        Tool::Brush* brs = dynamic_cast<Tool::Brush*>(curr_tool_);
        static float bcol[4] = {1.0f,1.0f,1.0f,1.0f};
        ImGui::ColorPicker4("BrushColor", bcol, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayRGB);
        Col_RGBA color = {
            (unsigned char)(bcol[0] * 0xff),
            (unsigned char)(bcol[1] * 0xff),
            (unsigned char)(bcol[2] * 0xff),
            (unsigned char)(bcol[3] * 0xff)
        };
        if (brs->col_ != color) brs->col_ = color;
    }
}
