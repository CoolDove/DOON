#include "Application.h"
#include <Core/Serialize.h>
#include "DoveLog.hpp"
#include <Core/Tool/Brush.h>
#include <Core/Config.h>


void Application::register_app_actions() {
    if (action_list_.get() == nullptr)
        DLOG_ERROR("ActionList hasnt been created, registering app actions failed");

    ActionList* alist = action_list_.get();

    alist->register_action("undo", action_undo);
    alist->register_action("redo", action_redo);
    alist->register_action("save_current_scene", action_save_current_scene);
    alist->register_action("load_config", action_load_config);
}

void Application::action_undo() {
    if (instance_ == nullptr || instance_->curr_scene_ == nullptr) return;
    instance_->curr_scene_->get_history_sys()->undo();
}

void Application::action_redo() {
    if (instance_ == nullptr || instance_->curr_scene_ == nullptr) return;
    instance_->curr_scene_->get_history_sys()->redo();
}

void Application::action_save_current_scene() {
    if (instance_ == nullptr || instance_->curr_scene_ == nullptr) return;
    DooWriter writer(instance_->curr_scene_);
    writer.write("d:/paintings/test.doo");
}

void Application::action_load_config() {
    using namespace Tool;
    auto app = Application::instance_;

    Config config("./res/.doon");

    bool end = false;

    while (!end) {
        char* namebuf = (char*)malloc(256 * sizeof(char));
        char* typebuf = (char*)malloc(256 * sizeof(char));
        bool iseof = false;
        SettingPair pairs = config.parse_settings(&iseof, namebuf, typebuf);
        if (!strcmp(typebuf, "brush")) {
            Brush* brush = Brush::ConfigMake(&pairs);
            if (brush != nullptr) {
                app->add_brush(namebuf, brush);
                DLOG_DEBUG("brush \"%s\" loaded.", namebuf);
            } else {
                DLOG_ERROR("failed to load brush: %s", namebuf);
            }
        } else if (!strcmp(typebuf, "keymap")) {
            // TODO: finish this
            ActionList* actions = app->action_list_.get();
            if (actions->config_keymap(&pairs, namebuf)) {
                // success
            } else {
                DLOG_ERROR("failed to load keymap, will use default keymap");
            }
            
        } else if (!strcmp(typebuf, "blendmode")) {
            // TODO: finish this
        } else {
            DLOG_ERROR("unrecognized setting type: %s, only supports: brush, keymap, blendmode", typebuf);
        }
        free(namebuf);
        free(typebuf);
        if (iseof) end = true;
    }
}
