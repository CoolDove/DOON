#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <thread>
#include <Windows.h>

#include "Scene.h"
#include "Tool/Tools.h"

#include <Core/Action.h>
#include <Core/DOONRes.h>

#define WGL_CONTEXT_MAJOR_VERSION_ARB           	0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB           	0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB             	0x2093
#define WGL_CONTEXT_FLAGS_ARB                   	0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB            	0x9126
// bits for WGL_CONTEXT_FLAGS_ARB
#define WGL_CONTEXT_DEBUG_BIT_ARB               	0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB  	0x0002
// bits for WGL_CONTEXT_PROFILE_MASK_ARB
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB        	0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 	0x00000002

using namespace std;

class Renderer;

class Application {
public:
    static Application* instance_;
    Application(HINSTANCE _instance, HINSTANCE _prev_instance, char* _cmd_line, int _show_code);
    ~Application();
    void run();
    
    void handle_event();
    void render_ui();

    void change_scene(const std::string& _name);

    Scene* add_scene(const std::string& path);
    Scene* add_scene(const std::string& name, uint32_t width, uint32_t height, Col_RGBA col);
    void remove_scene(const std::string& name);
    void clear_scenes();

    void add_brush(const std::string& name, Tool::Brush* p_brush);

    Scene* curr_scene_;
    unordered_map<string, Scene*> scenes_;

    unordered_map<string, Tool::Brush*> brushes_;
    
    // @Tools:
    Tool::Tool*             curr_tool_;
    struct {
    // unique_ptr<Tool::Brush> brush;
    // Tool::Brush*      brush;
    // ...
    } tools_;

    unique_ptr<Renderer>    renderer_;
    unique_ptr<DOONRes>     RES;

public:// window
    bool    inited_;
    HWND    window_;
    HDC     device_context_;
    unique_ptr<ActionList> action_list_;

    struct WindowInfo {
        int posx;
        int posy;
        int width;
        int height;
    } window_info_;

public:
    GLuint buf_tex_;

public: 
    void register_app_actions();
    static void action_undo();
    static void action_redo();
    static void action_save_current_scene();
    static void action_save_current_scene_as();
    static void action_open_file();
    static void action_load_config();

private:
    void init_dlog();
    void init_window(HINSTANCE _instance, HINSTANCE _prev_instance, char* _cmd_line, int _show_code);
    void init_imgui();

    void init_tools();
    void init_tablet();
private:
    void gui_BrushChooser();
};
