﻿#include "Application.h"
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
    if (!instance_) 
        instance_ = this;

    init_dlog();
    init_window(_instance, _prev_instance, _cmd_line, _show_code);
    renderer_ = make_unique<Renderer>(this);
    init_imgui();
    init_tablet();

    inited_ = true;

    long clock = std::clock();
    scenes_["void"]  = make_unique<Scene>(0x00000000);
    // scenes_["jko"]   = make_unique<Scene>("./res/textures/jko.png");
    // scenes_["anji"]  = make_unique<Scene>("./res/textures/anji.png");
    scenes_["alp"]   = make_unique<Scene>("./res/textures/alp.png");

    if (scenes_.size() == 0) {
        scenes_["void"]  = make_unique<Scene>(0x00000000);
    }
    
    curr_scene_ = scenes_.begin()->second.get();
    clock = std::clock() - clock;

    DLOG_TRACE("scene loaded, takes %ldms", clock);

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
            render_ui();
            renderer_->render();
        } else {
            break;
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();

    ImGui::DestroyContext();
}

void Application::render_ui() {
    DGL::Camera* cam = &curr_scene_->camera_;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGuiIO& io = ImGui::GetIO();

    ImGui::NewFrame();
    if (ImGui::Begin("temp")) {
        float cam_region = 0.5f * glm::max(curr_scene_->image_.info_.width, curr_scene_->image_.info_.height);
        ImGui::DragFloat2("cam_pos", (float*)&cam->position_, 1.0f, -cam_region, cam_region);
        ImGui::DragFloat("cam_size", &cam->size_, 0.1f, 0.1f, 10.0f);
        if (dynamic_cast<Tool::Brush*>(curr_tool_)) {
            Tool::Brush* brs = dynamic_cast<Tool::Brush*>(curr_tool_);
            // ImGui::ColorEdit4("brush_col", brs->col_, ImGuiColorEditFlags_AlphaBar);
            ImGui::DragIntRange2("brush_size", &brs->size_min_, &brs->size_max_, 1, 0, 100);
        }
        
        ImGui::End();
    }

    ImGui::SetNextWindowPos({1, window_info_.height - 1.0f}, 0, {0.0f, 1.0f});
    {
        if (ImGui::Begin("scene tab", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize)) {
            for (auto ite = scenes_.begin(); ite != scenes_.end(); ite++)
            {
                if (ImGui::Selectable(ite->first.c_str())) {
                    change_scene(ite->first);
                }
            }
            ImGui::End();
        }
    }

    ImGui::SetNextWindowPos({1, 1});
    if (ImGui::Begin("info", nullptr, 
                     ImGuiWindowFlags_NoInputs|
                     ImGuiWindowFlags_NoTitleBar|
                     ImGuiWindowFlags_AlwaysAutoResize)) 
    {
        ImGui::LabelText("mouse pos", "[%.0f, %.0f] ", io.MousePos.x, io.MousePos.y);
        ImGui::Selectable("imgui capturing mouse", io.WantCaptureMouse);
        ImGui::Selectable("imgui capturing keyboard", io.WantCaptureKeyboard);
        ImGui::End();
    }

    ImGui::EndFrame();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SwapBuffers(device_context_);
}

void Application::change_scene(const std::string& _name) {
    if (scenes_.find(_name) != scenes_.end()) {
        curr_scene_ = scenes_[_name].get();
        renderer_->create_gl_image();
    }
}

void Application::init_dlog() {
    DLOG_ON_PUSH = [](const Dove::LogMsg& _msg){
        OutputDebugString(_msg.to_string(Dove::DMSG_FLAG_SIMPLE | Dove::DMSG_FLAG_FILE | Dove::DMSG_FLAG_LINE).c_str());
        OutputDebugString("\n");
    };

    DLOG_INIT;
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
                             WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_THICKFRAME, 
                            //  WS_SIZEBOX | WS_VISIBLE | WS_CAPTION | WS_SYSMENU, 
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
    ImGui::StyleColorsClassic();
    ImFont* font = io.Fonts->AddFontFromFileTTF("res/fonts/FiraCode.ttf", 20.0f);
    io.FontDefault = font;
    io.FontGlobalScale = 0.8f;

    // imgui style
    auto colfom = [](unsigned int _col)->ImVec4{
        unsigned char col[4] = {
            (_col & 0xff000000)>>24,
            (_col & 0x00ff0000)>>16,
            (_col & 0x0000ff00)>> 8,
            (_col & 0x000000ff)>> 0,
        };
        return {(float)col[0]/255.0f, (float)col[1]/255.0f, (float)col[2]/255.0f, (float)col[3]/255.0f};
    };

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize,  0);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize,  0);
    ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize,  1);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,    3);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,   3);

    ImGui::PushStyleColor(ImGuiCol_TitleBg,                 colfom(0x212121ee));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive,           colfom(0x6d9886ff));
    ImGui::PushStyleColor(ImGuiCol_TitleBgCollapsed,        colfom(0x6d9886a0));

    ImGui::PushStyleColor(ImGuiCol_MenuBarBg,               colfom(0x323232e0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg,                colfom(0x212121ee));

    ImGui::PushStyleColor(ImGuiCol_FrameBg,                 colfom(0x6e6e6e62));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,           colfom(0x7e7e7e62));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered,          colfom(0x80808079));


    ImGui::PushStyleColor(ImGuiCol_Text,                    colfom(0xf6f6f6ee));
    ImGui::PushStyleColor(ImGuiCol_TextDisabled,            colfom(0xd9cab3ef));
    ImGui::PushStyleColor(ImGuiCol_TextSelectedBg,          colfom(0xf6f6f6aa));

    ImGui::PushStyleColor(ImGuiCol_ScrollbarBg,             colfom(0x323232e0));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab,           colfom(0x6d9886ee));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive,     colfom(0x9dd9afff));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered,    colfom(0x8dc99aff));

    ImGui::PushStyleColor(ImGuiCol_Header,                  colfom(0x6d9886ff));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive,            colfom(0x7dad90ff));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered,           colfom(0x7dad90ff));

    ImGui::PushStyleColor(ImGuiCol_Tab,                     colfom(0x6d9886ff));
    ImGui::PushStyleColor(ImGuiCol_TabActive,               colfom(0x7dad90ff));
    ImGui::PushStyleColor(ImGuiCol_TabHovered,              colfom(0x7dad90ff));
    ImGui::PushStyleColor(ImGuiCol_TabUnfocused,            colfom(0x6d9886ff));
    ImGui::PushStyleColor(ImGuiCol_TabUnfocusedActive,      colfom(0x6d9886ff));

    ImGui::PushStyleColor(ImGuiCol_Button,                  colfom(0x6d9886ff));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,            colfom(0x7dad90ff));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,           colfom(0x9dbd90ff));

    ImGui::PushStyleColor(ImGuiCol_ResizeGrip,              colfom(0xf2f2f277));
    ImGui::PushStyleColor(ImGuiCol_ResizeGripActive,        colfom(0xf2f2f2ff));
    ImGui::PushStyleColor(ImGuiCol_ResizeGripHovered,       colfom(0xf2f2f2dd));

    ImGui::PushStyleColor(ImGuiCol_SliderGrab,              colfom(0xf2f2f288));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive,        colfom(0x6d9886ff));

    ImGui::PushStyleColor(ImGuiCol_CheckMark,               colfom(0xf2f2f2ee));

    DLOG_INFO("ImGui has been initialized");
}

void Application::init_tablet() {
    EnableMouseInPointer(false);
}