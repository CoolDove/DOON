#include "Application.h"
#include <Core/Serialize.h>
#include "DoveLog.hpp"
#include <Core/Tool/Brush.h>
#include <Core/Config.h>
#include <Base/Win_FileChooser.h>
#include <stb_image/stb_image_write.h>


void Application::register_app_actions() {
    if (action_list_.get() == nullptr)
        DLOG_ERROR("ActionList hasnt been created, registering app actions failed");

    ActionList* alist = action_list_.get();

    alist->register_action("undo", action_undo);
    alist->register_action("redo", action_redo);

    alist->register_action("save_current_scene", action_save_current_scene);
    alist->register_action("save_current_scene_as", action_save_current_scene_as);
    alist->register_action("export_current_scene", action_export_current_scene);

    alist->register_action("open_file", action_open_file);
    alist->register_action("new_scene", action_new_scene);
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
    auto app = Application::instance_;
    Scene* scn = app->curr_scene_;
    if (scn == nullptr) return;

    if (scn->save_path_ != "") {
        DooWriter writer(app->curr_scene_);
        writer.write(scn->save_path_.c_str());
    } else {
        action_save_current_scene_as();
    }
}

void Application::action_save_current_scene_as() {
    auto app = Application::instance_;
    Scene* scn = app->curr_scene_;
    if (scn == nullptr) return;

    std::string file = OS::choose_file_save();
    if (file != "") {
        scn->save_path_ = file;
        DooWriter writer(app->curr_scene_);
        writer.write(scn->save_path_.c_str());
    }
}

void Application::action_export_current_scene() {
    auto app = Application::instance_;
    Scene* scn = app->curr_scene_;
    if (scn == nullptr) return;
    std::string file = OS::choose_file_export();
    if (file != "") {
        DLOG_DEBUG("export to :%s", file.c_str());
        auto tex = scn->get_composed_texture();
        auto pixels = tex->mem_fetch();
        stbi_write_png(file.c_str(), scn->info_.width, scn->info_.height, 4, pixels, 0);
        tex->mem_release();
    }
}

void Application::action_open_file() {
    using namespace std;
    auto app = Application::instance_;
    string filename = OS::choose_file_open();

    if (filename != "") {
        app->curr_scene_ = app->add_scene(filename);
        std::string ext = filename.substr(filename.size() - 3 ,filename.size());
        if (ext == "doo") app->curr_scene_->save_path_ = filename;
    }
}

void Application::action_new_scene() {
    // TODO: action new scene 
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
            ActionList* actions = app->action_list_.get();
            if (actions->config_keymap(&pairs, namebuf)) {
                // success
            } else {
                DLOG_ERROR("failed to load keymap, will use default keymap");
            }
            
        } else if (!strcmp(typebuf, "blendmode")) {
            // TODO: blend mode load config
        } else {
            // nothing
        }
        free(namebuf);
        free(typebuf);
        if (iseof) end = true;
    }
}
