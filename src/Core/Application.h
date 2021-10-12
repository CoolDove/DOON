#pragma once
#include <unordered_map>
#include <string>
#include <memory>
#include <thread>
#include <Windows.h>

#include "Scene.h"

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

class Application {
public:
    Application(HINSTANCE _instance, HINSTANCE _prev_instance, char* _cmd_line, int _show_code);
    ~Application();
    void run();

    static Application* get_instance() { return instance_; };
    static Application* instance_;

public:
    struct WindowInfo {
        int width;
        int height;
    } window_info_;

    struct OpenGLInfo {
        string version;
        string vendor;
        string renderer;
        string shading_lang_version;
    } gl_info;

    bool inited_;
public:
    void handle_event();
    void render();

    // this is a temporary function, we will have a Painter class or something for this
    void draw_circle(Image* _img, int _x, int _y, int _r, unsigned int _col);

public:
    unordered_map<string, unique_ptr<Scene>> scenes_;
    Scene* curr_scene_;

    unique_ptr<DGL::Shader> shader_;

    GLuint img_id;

    HWND    window_;
    HDC     device_context_;
    HGLRC   gl_context_;
    
private:
    void init_dlog();
    void init_window(HINSTANCE _instance, HINSTANCE _prev_instance, char* _cmd_line, int _show_code);
    void init_opengl();
    void init_imgui();
};
Application* get_app();