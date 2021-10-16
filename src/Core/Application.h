#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <thread>
#include <Windows.h>

#include "Scene.h"
#include "Tool/Tools.h"

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
    void render();

    void change_scene(const std::string& _name);


    Scene* curr_scene_;
    unordered_map<string, unique_ptr<Scene>> scenes_;

/* TOOLS */
    Tool::Tool*             curr_tool_;
    struct {
    unique_ptr<Tool::Brush> brush;
    // ...
    } tools_;
/* TOOLS */

    unique_ptr<Renderer>    renderer_;

public:// window
    bool    inited_;
    HWND    window_;
    HDC     device_context_;

    struct WindowInfo {
        int posx;
        int posy;
        int width;
        int height;
    } window_info_;

private:
    void init_dlog();
    void init_window(HINSTANCE _instance, HINSTANCE _prev_instance, char* _cmd_line, int _show_code);
    void init_imgui();

    void init_tools();
    void init_tablet();
};