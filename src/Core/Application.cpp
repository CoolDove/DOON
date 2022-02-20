#include "Application.h"
#include "DoveLog.hpp"
#include "Renderer.h"
#include <DGLCore/DGLCore.h>
#include <Core/Action.h>
#include <Core/Command_Brush.h>

#include <algorithm>
#include <assert.h>
#include <ctime>
#include <time.h>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <Core/Input.h>

#include <glad/glad.h>
#include <gl/GL.h>

// @Temporary: test reading binary files
#include <Core/Serialize.h>
#include <Core/Config.h>

using wglCreateContextAttribsARB_t = HGLRC (WINAPI *) (HDC hDC, HGLRC hshareContext, const int *attribList);

Application* Application::instance_ = nullptr;

// TODO: try to process input message in another thread, to get preciser mouse motion trace
Application::Application(HINSTANCE _instance, HINSTANCE _prev_instance, char* _cmd_line, int _show_code) 
:   window_info_{0, 0},
    inited_(false)
{
    using namespace Tool;
    if (!instance_) 
        instance_ = this;

    // NOTE:
    // initialization order:
    // dlog >> doonres >> window >> render(>>opengl) >> imgui >> tablet >> scene >> tools

    init_dlog();

    init_window(_instance, _prev_instance, _cmd_line, _show_code);
    renderer_ = make_unique<Renderer>(this);
    init_imgui();
    init_tablet();

    inited_ = true;

    // @LoadResource:
    RES = std::make_unique<DOONRes>();
    RES->SetResourcePath("./res");
    RES->LoadResourcesPath();


    long clock = std::clock();
    // scenes_["anji"]  =   make_unique<Scene>("./res/textures/anji.png");
    // scenes_["dooload"] = make_unique<Scene>("d:/paintings/test.doo");
    // scenes_["big"] =     make_unique<Scene>(512, 512, Col_RGBA{0x43, 0x32, 0x64, 0xff});

    if (scenes_.size() == 0) {
        add_scene("void", 2048, 2048, Col_RGBA{0x00, 0x00, 0x00, 0x00});
    }
    
    curr_scene_ = scenes_.begin()->second;
    clock = std::clock() - clock;

    DLOG_TRACE("scene loaded, takes %ldms", clock);

    // init tools
    add_brush("default", new Brush(this));
    curr_tool_ = brushes_["default"];

    renderer_->init();

    // @ActionList:
    action_list_ = std::make_unique<ActionList>();
    register_app_actions();

    Application::action_load_config();
}

Application::~Application() {
    for (auto ite = brushes_.begin(); ite != brushes_.end(); ite++) {
        delete ite->second;
    }
}

void Application::run() {
    MSG msg;
    while (BOOL result = GetMessage(&msg, nullptr, 0, 0)) {
        if (result > 0) {
            // @Update:
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (curr_scene_) curr_scene_->on_update();
            if (curr_tool_)  curr_tool_->on_update();

            renderer_->render();
            render_ui();
        } else {
            break;
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();

    ImGui::DestroyContext();
}

Scene* Application::add_scene(const std::string& path) {
    remove_scene(path);
    scenes_[path] = new Scene(path.c_str());
    return scenes_[path];
}

Scene* Application::add_scene(const std::string& name, uint32_t width, uint32_t height, Col_RGBA col) {
    remove_scene(name);
    scenes_[name] = new Scene(width, height, col);
    return scenes_[name];
}
void Application::remove_scene(const std::string& name) {
    if (name == "void") return;
    auto f = scenes_.find(name);
    if (f != scenes_.end()) {
        delete f->second;
        scenes_.erase(f);
    }
}

void Application::clear_scenes() {
    for (auto ite = scenes_.begin(); ite != scenes_.end(); ite++) {
        if (ite->first != "void") {
            delete ite->second;
            scenes_.erase(ite);
        }
    }
}

void Application::render_ui() {
    DGL::Camera* cam = &curr_scene_->camera_;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGuiIO& io = ImGui::GetIO();

    ImGui::NewFrame();
    {
        if (ImGui::Begin("panel")) {
            gui_BrushChooser();
            
            if (ImGui::CollapsingHeader("cam")) {
                float cam_region = 0.5f * glm::max(curr_scene_->info_.width, curr_scene_->info_.height);
                ImGui::DragFloat2("cam_pos", (float*)&cam->position_, 1.0f, -cam_region, cam_region);
                ImGui::DragFloat("cam_size", &cam->size_, 0.1f, 0.1f, 10.0f);
            }

            if (ImGui::CollapsingHeader("tool")) {
                if (dynamic_cast<Tool::Brush*>(curr_tool_)) {
                    Tool::Brush* brs = dynamic_cast<Tool::Brush*>(curr_tool_);
                    static float bcol[4] = {1.0f,1.0f,1.0f,1.0f};
                    ImGui::ColorEdit4("brush_col", bcol, ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_DisplayRGB);
                    Col_RGBA color = {
                        (unsigned char)(bcol[0] * 0xff),
                        (unsigned char)(bcol[1] * 0xff),
                        (unsigned char)(bcol[2] * 0xff),
                        (unsigned char)(bcol[3] * 0xff)
                    };

                    if (brs->col_ != color) brs->col_ = color;

                    ImGui::DragInt("brush_size_max", &brs->size_max_, 0.1f, 1, 7000);
                    ImGui::DragFloat("brush_size_min", &brs->size_min_scale_, 0.01f, 0.0f, 1.0f);
                }
            }

            if (ImGui::CollapsingHeader("scene")) {
                if (ImGui::Button("add layer")) {
                    curr_scene_->add_layer(Col_RGBA{0x00, 0x00, 0x00, 0x00});
                }
                ImGui::BeginGroup();
                for (auto ite = curr_scene_->layers_.rbegin(); ite != curr_scene_->layers_.rend(); ite++)
                {
                    if (curr_scene_->get_curr_layer() == ite->get()) {
                        ImGui::Bullet();
                        ImGui::SameLine();
                    }
                    if (ImGui::Button(ite->get()->info_.name.c_str())) {
                        curr_scene_->change_layer(ite->get());
                    }
                }
                ImGui::EndGroup();
            }
            ImGui::End();
        }
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
        ImGui::LabelText("render interval", "delt: %3.1fms \tFPS: %.0f", io.DeltaTime * 1000.0f, 1.0f / io.DeltaTime);
        ImGui::LabelText("mouse pos", "[%.0f, %.0f] ", io.MousePos.x, io.MousePos.y);
        ImGui::Selectable("imgui capturing mouse", io.WantCaptureMouse);
        ImGui::Selectable("imgui capturing keyboard", io.WantCaptureKeyboard);

        ImGui::BeginGroup();
        ImGui::LabelText("canvas size",  "-%d * %d-", curr_scene_->info_.width, curr_scene_->info_.height);
        ImGui::EndGroup();

        auto* calls = &action_list_->key_pages_["default"];
        auto* actions = &action_list_->action_map_;

        for (auto ite = calls->begin(); ite != calls->end(); ite++) {
            auto action_ite = actions->find(ite->second);
            if (action_ite != actions->end()) {
                std::string action_name = to_string(ite->first);
                ImGui::LabelText(action_ite->first.c_str(), action_name.c_str());
            }
        }

        ImGui::End();
    }

    ImGui::EndFrame();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SwapBuffers(device_context_);
}

void Application::change_scene(const std::string& _name) {
    if (scenes_.find(_name) != scenes_.end()) {
        curr_scene_ = scenes_[_name];
    }
}

void Application::init_dlog() {
    DLOG_ON_PUSH = [](const Dove::LogMsg& _msg){
        OutputDebugString(_msg.to_string(Dove::DMSG_FLAG_SIMPLE | Dove::DMSG_FLAG_FILE | Dove::DMSG_FLAG_LINE).c_str());
        OutputDebugString("\n");
    };

    DLOG_TRACE("dlog inited");

    DLOG_INIT;
}

void Application::add_brush(const std::string& name, Tool::Brush* p_brush) {
    if (p_brush == nullptr) return;
    bool set_currtool = false;
    if (brushes_.find(name) != brushes_.end()) {
        if (curr_tool_ == brushes_[name]) set_currtool = true;
        delete brushes_[name];
    }
    curr_tool_ = brushes_[name] = p_brush;
}

// @region: INIT
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
                             // WS_SIZEBOX | WS_VISIBLE | WS_CAPTION | WS_SYSMENU, 
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
    // @why:
    // ImGui_ImplWin32_EnableDpiAwareness();                                        
    // i dont know why this function is called in the imgui example,                
    // but for my situation,                                                        
    // calling this causes an offset of my mouse position depends on the window size
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
            (unsigned char)((_col & 0xff000000)>>24),
            (unsigned char)((_col & 0x00ff0000)>>16),
            (unsigned char)((_col & 0x0000ff00)>> 8),
            (unsigned char)((_col & 0x000000ff)>> 0),
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
