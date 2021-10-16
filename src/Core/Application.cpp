#include "Application.h"
#include "DoveLog.hpp"
#include "Renderer.h"

#include <assert.h>
#include <time.h>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <Core/Input.h>

#include <glad/glad.h>
#include <gl/GL.h>

using wglCreateContextAttribsARB_t = HGLRC (WINAPI *) (HDC hDC, HGLRC hshareContext, const int *attribList);

Application* Application::instance_ = nullptr;

Application::Application(HINSTANCE _instance, HINSTANCE _prev_instance, char* _cmd_line, int _show_code) 
:   window_info_{0, 0},
    inited_(false)
{
    instance_ = this;

    init_dlog();
    init_window(_instance, _prev_instance, _cmd_line, _show_code);
    renderer_ = make_unique<Renderer>(this);
    init_imgui();
    init_tablet();


    inited_ = true;

    scenes_["jko"]    = make_unique<Scene>("./res/textures/jko.png");
    scenes_["anji"]   = make_unique<Scene>("./res/textures/anji.png");
    curr_scene_       = scenes_["anji"].get();

    DLOG_TRACE("scene loaded");

    // init tools
    tools_.brush = make_unique<Tool::Brush>(this);
    tools_.brush->on_init();
    curr_tool_ = tools_.brush.get();
    curr_tool_->on_activate();

    renderer_->init();
}

Application::~Application() {

}

void Application::run() {
    MSG msg;
    while (BOOL result = GetMessage(&msg, nullptr, 0, 0)) {
        if (result > 0) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            render();
            renderer_->render();
        } else {
            break;
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();

    ImGui::DestroyContext();
}

void Application::render() {

    DGL::Camera* cam = &curr_scene_->camera_;

    // glm::mat4 view = cam->calc_view();

    // int width  = get_app()->window_info_.width;
    // int height = get_app()->window_info_.height;
    // glm::mat4 proj = cam->calc_proj(width, height);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGuiIO& io = ImGui::GetIO();

    ImGui::NewFrame();
    if (ImGui::Begin("temp")) {
        ImGui::DragFloat2("cam_pos", (float*)&cam->position_, 0.1f, -10.0f, 10.0f);
        ImGui::DragFloat("cam_size", &cam->size_, 0.1f, 0.1f, 10.0f);
        if (dynamic_cast<Tool::Brush*>(curr_tool_)) {
            ImGui::ColorPicker3("brush_col", dynamic_cast<Tool::Brush*>(curr_tool_)->col_);
        }
        ImGui::End();
    }
    ImGui::SetNextWindowPos({1, 1});
    if (ImGui::Begin("info", nullptr, 
                     ImGuiWindowFlags_NoInputs|
                     ImGuiWindowFlags_NoTitleBar|
                     ImGuiWindowFlags_AlwaysAutoResize)) 
    {
        ImGui::LabelText("mouse pos", "[%.0f, %.0f] ", io.MousePos.x, io.MousePos.y);
        // ImGui::LabelText("mouse pos", "[%.0f, %.0f] ", io.MousePos.x, io.MousePos.y);
        ImGui::Selectable("imgui capturing mouse", io.WantCaptureMouse);
        ImGui::Selectable("imgui capturing keyboard", io.WantCaptureKeyboard);
        ImGui::End();
    }

    ImGui::EndFrame();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SwapBuffers(device_context_);
}

void Application::init_dlog() {
    DLOG_ON_PUSH = [](const Dove::LogMsg& _msg){
        OutputDebugString(_msg.to_string(Dove::DMSG_FLAG_SIMPLE | Dove::DMSG_FLAG_FILE | Dove::DMSG_FLAG_LINE).c_str());
        OutputDebugString("\n");
    };

    DLOG_INIT;
}

Application* get_app() {
    return Application::get_instance();
}

// region: INIT
void Application::init_window(HINSTANCE _instance, HINSTANCE _prev_instance, char* _cmd_line, int _show_code) {
    WNDCLASS wnd_class = {};
    wnd_class.style = CS_HREDRAW | CS_VREDRAW;
    wnd_class.lpfnWndProc = Input::wnd_proc;
    wnd_class.hInstance = _instance;
    wnd_class.lpszClassName = "DOONWindowClass";

    RegisterClass(&wnd_class);

    window_ = CreateWindowEx(0,
                             wnd_class.lpszClassName, 
                             "DOON", 
                             WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
                             CW_USEDEFAULT, 
                             CW_USEDEFAULT,
                             800, 
                             600,
                             nullptr, 
                             nullptr,
                             _instance,
                             nullptr);

    if (!window_) {
        MessageBox(nullptr, "error occured while creating windows", "error", MB_OK);
        return;
    }

    device_context_ = GetDC(window_);
}

void Application::init_imgui() {
    /*──────────────────────────────────────────────────────────────────────────────┐
    │ ImGui_ImplWin32_EnableDpiAwareness();                                         │
    │ i dont know why this function is called in the imgui example,                 │
    │ but for my situation,                                                         │
    │ calling this causes an offset of my mouse position depends on the window size │
    └──────────────────────────────────────────────────────────────────────────────*/
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui_ImplWin32_Init(window_);
    Input::imgui_proc = ImGui_ImplWin32_WndProcHandler;
    ImGui_ImplOpenGL3_Init(nullptr);

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    // ImFont* font = io.Fonts->AddFontFromFileTTF("...", 14.0f);

    DLOG_INFO("ImGui has been initialized");
}

void Application::init_tablet() {
    EnableMouseInPointer(false);
}