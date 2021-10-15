#include "Application.h"
#include "DoveLog.hpp"

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
    gl_info_{},
    inited_(false)
{
    instance_ = this;

    init_dlog();
    init_window(_instance, _prev_instance, _cmd_line, _show_code);
    init_imgui();
    init_tablet();

    inited_ = true;

    scenes_["jko"]  = make_unique<Scene>("./res/textures/jko.png");
    scenes_["test"] = make_unique<Scene>("./res/textures/test.png");
    curr_scene_ = scenes_["jko"].get();

    DLOG_TRACE("scene loaded");
    
    shader_ = std::make_unique<DGL::Shader>();
    shader_->load("./res/shaders/base.vert", "./res/shaders/base.frag");
    shader_->bind();

    // init tools
    tools_.brush = make_unique<Tool::Brush>(this);
    tools_.brush->on_init();
    curr_tool_ = tools_.brush.get();
    curr_tool_->on_activate();
}

Application::~Application() {

}

void Application::run() {
    glCreateTextures(GL_TEXTURE_2D, 1, &img_id);
    // TODO: move this part to somewhere else to realloc memory while switch current scene
    //       or just move into renderer
    glTextureStorage2D(img_id, 1, GL_RGBA12, 
                       curr_scene_->image_.info_.width, 
                       curr_scene_->image_.info_.height);

    glBindTextureUnit(0, img_id);
    glUniform1i(glGetUniformLocation(shader_->get_id(), "_tex"), 0);

    MSG msg;
    while (BOOL result = GetMessage(&msg, nullptr, 0, 0)) {
        if (result > 0) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            render();
        } else {
            break;
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();

    ImGui::DestroyContext();
}

void Application::render() {
    // background color
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    glClear(GL_COLOR_BUFFER_BIT);

    glTextureSubImage2D(img_id, 0, 0, 0, 
                        curr_scene_->image_.info_.width,
                        curr_scene_->image_.info_.height,
                        GL_RGBA, 
                        GL_UNSIGNED_BYTE, 
                        curr_scene_->image_.pixels_);	

    glBindTexture(GL_TEXTURE_2D, img_id);

    int uid_view_matrix = glGetUniformLocation(shader_->get_id(), "_view");
    int uid_proj_matrix = glGetUniformLocation(shader_->get_id(), "_proj");

    DGL::Camera* cam = &curr_scene_->camera_;

    glm::mat4 view = cam->calc_view();

    int width  = get_app()->window_info_.width;
    int height = get_app()->window_info_.height;
    glm::mat4 proj = cam->calc_proj(width, height);

    glUniformMatrix4fv(uid_view_matrix, 1, false, &view[0][0]);
    glUniformMatrix4fv(uid_proj_matrix, 1, false, &proj[0][0]);

    curr_scene_->batch_.draw_batch();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();

    ImGui::NewFrame();
    if (ImGui::Begin("camera")) {
        ImGui::DragFloat2("pos", (float*)&cam->position_, 0.1f, -10.0f, 10.0f);
        ImGui::DragFloat("size", &cam->size_, 0.1f, 0.1f, 10.0f);
        ImGui::End();
    }
    DLOG_TRACE("ImGui::MousePos:%f,%f", ImGui::GetMousePos().x, ImGui::GetMousePos().y);

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

    // pixel format
    PIXELFORMATDESCRIPTOR desired_pixel_format = {};
    desired_pixel_format.nSize      = sizeof(desired_pixel_format);
    desired_pixel_format.nVersion   = 1;
    desired_pixel_format.dwFlags    = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
    desired_pixel_format.iPixelType = PFD_TYPE_RGBA;
    desired_pixel_format.cColorBits = 32;
    desired_pixel_format.cAlphaBits = 8;

    PIXELFORMATDESCRIPTOR suggested_pixel_format = {};
    int pixel_format_index = ChoosePixelFormat(device_context_, &desired_pixel_format);

    DescribePixelFormat(device_context_,
                        pixel_format_index,
                        sizeof(suggested_pixel_format),
                        &suggested_pixel_format);

    SetPixelFormat(device_context_, pixel_format_index, &suggested_pixel_format);

    gl_context_ = wglCreateContext(device_context_);
    wglMakeCurrent(device_context_, gl_context_);

    gladLoadGL();
    init_opengl();

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    SwapBuffers(device_context_);
}

void Application::init_opengl() {
    gl_info_.version              = (char*)glGetString(GL_VERSION);
    gl_info_.vendor               = (char*)glGetString(GL_VENDOR);
    gl_info_.renderer             = (char*)glGetString(GL_RENDERER);
    gl_info_.shading_lang_version = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    wglCreateContextAttribsARB_t wglCreateContextAttribsARB
        = (wglCreateContextAttribsARB_t)wglGetProcAddress("wglCreateContextAttribsARB");
    if (wglCreateContextAttribsARB) {
        HGLRC share_context = 0;
        int attrib_list[] = {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
            WGL_CONTEXT_MINOR_VERSION_ARB, 5,
#ifdef DEBUG
            WGL_CONTEXT_FLAGS_ARB,         WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
            WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
            0
        };
        HGLRC modern_glrc = wglCreateContextAttribsARB(device_context_, share_context, attrib_list);
        if (modern_glrc) {
            wglMakeCurrent(device_context_, modern_glrc);
            DLOG_TRACE("switch to modern OpenGL Render Context");
            gl_context_ = modern_glrc;
        }
    }
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