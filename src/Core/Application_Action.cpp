#include "Application.h"
#include <Core/Serialize.h>
#include "DoveLog.hpp"
#include <Core/Config.h>

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
}

void Application::action_load_config() {
    Config config("./res/.doon");


    bool end = false;
    while (!end) {
        char* namebuf = (char*)malloc(256 * sizeof(char));
        char* typebuf = (char*)malloc(256 * sizeof(char));
        bool iseof = false;
        SettingPair pairs = config.parse_settings(&iseof, namebuf, typebuf);
        free(namebuf);
        free(typebuf);
        if (iseof) end = true;
    }
}
