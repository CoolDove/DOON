#include "Application.h"
#include <Core/Serialize.h>
#include "DoveLog.hpp"

void Application::action_undo() {
    if (instance_ == nullptr || instance_->curr_scene_ == nullptr) return;
    instance_->curr_scene_->get_history_sys()->undo();
}

void Application::action_redo() {
    if (instance_ == nullptr || instance_->curr_scene_ == nullptr) return;
    instance_->curr_scene_->get_history_sys()->redo();
}

void Application::action_save() {
    if (instance_ == nullptr || instance_->curr_scene_ == nullptr) return;
    DooWriter writer(instance_->curr_scene_);

    writer.write("d:/paintings/test.doo");

    DLOG_DEBUG("saved");
}
