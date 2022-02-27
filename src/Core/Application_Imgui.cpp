#include "Application.h"
#include <imgui/imgui.h>

void Application::gui_ToolsChooser() {
    if (ImGui::CollapsingHeader("Tools")) {
        if (ImGui::Selectable("ColorPicker", curr_tool_ == tools_.color_picker)) {
            choose_tool(tools_.color_picker, true);
        }
        // @Brushes:
        if (ImGui::TreeNode("Brushes")) {
            for (auto ite = brushes_.begin(); ite != brushes_.end(); ite++) {
                if (ImGui::Selectable(ite->first.c_str(), curr_tool_ == ite->second)) {
                    choose_tool(ite->second, false);
                }
            }
            ImGui::TreePop();
        }
    }
}

void Application::gui_ColorPicker() {
    // FIXME: i should move the current brush color into Scene or otherwhere we can get without a brush pointer.
    // @Temp: a temp fix
    if (ImGui::CollapsingHeader("Color")) {
        Tool::Brush* brs = nullptr;
        brs = dynamic_cast<Tool::Brush*>(curr_tool_);
        static float bcol[4] = {1.0f,1.0f,1.0f,1.0f};
        ImGui::ColorPicker4("BrushColor", bcol, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayRGB);
        Col_RGBA color = {
            (unsigned char)(bcol[0] * 0xff),
            (unsigned char)(bcol[1] * 0xff),
            (unsigned char)(bcol[2] * 0xff),
            (unsigned char)(bcol[3] * 0xff)
        };

        if (brs != nullptr && brs->col_ != color) brs->col_ = color;
    }
}
