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
