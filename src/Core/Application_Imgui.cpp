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
        // static float bcol[4] = {1.0f,1.0f,1.0f,1.0f};
        Scene* scn = Application::instance_->curr_scene_;

        ImGui::ColorPicker4("BrushColor", (float*)&scn->fbrush_color_, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayRGB);
    }
}
