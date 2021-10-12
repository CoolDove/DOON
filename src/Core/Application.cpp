#include "Application.h"
#include "DoveLog.hpp"

#include <assert.h>
#include <time.h>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <glad/glad.h>
#include <gl/GL.h>


using wglCreateContextAttribsARB_t = HGLRC (WINAPI *) (HDC hDC, HGLRC hshareContext, const int *attribList);

Application* Application::instance_ = nullptr;

Application::Application(HINSTANCE _instance, HINSTANCE _prev_instance, char* _cmd_line, int _show_code) 
:   window_info_{0, 0},
    gl_info{},
    inited_(false)
{
    instance_ = this;

    init_dlog();
    init_window(_instance, _prev_instance, _cmd_line, _show_code);
    init_imgui();

    inited_ = true;

    scenes_["jko"] = make_unique<Scene>("./res/textures/jko.png");
    scenes_["test"] = make_unique<Scene>("./res/textures/test.png");

    curr_scene_ = scenes_["jko"].get();

    DLOG_TRACE("scene loaded");
    
    shader_ = std::make_unique<DGL::Shader>();
    /*──────────┐
    │ load shader
    └──────────*/
    shader_->load("./res/shaders/base.vert", "./res/shaders/base.frag");
    shader_->bind();
}

Application::~Application() {

}

void Application::run() {
    glCreateTextures(GL_TEXTURE_2D, 1, &img_id);
    // TODO(dove): move this part to somewhere else to realloc memory while switch current scene
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
    glClearColor(0.8f, 0.4f, 0.1f, 1.0f);
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

    // TODO: window width and height
    int width  = get_app()->window_info_.width;
    int height = get_app()->window_info_.height;
    glm::mat4 proj = cam->calc_proj(width, height);

    glUniformMatrix4fv(uid_view_matrix, 1, false, &view[0][0]);
    glUniformMatrix4fv(uid_proj_matrix, 1, false, &proj[0][0]);

    curr_scene_->batch_.draw_batch();

    // imgui layer
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();

    ImGui::NewFrame();
    // some imgui rendering

    if (ImGui::Begin("camera")) {
        ImGui::DragFloat2("pos", (float*)&cam->position_, 0.1f, -10.0f, 10.0f);
        ImGui::DragFloat("size", &cam->size_, 0.1f, 0.1f, 10.0f);
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

LRESULT CALLBACK windows_proc(HWND _window, UINT _message, WPARAM _w_param, LPARAM _l_param) {
    if (ImGui_ImplWin32_WndProcHandler(_window, _message, _w_param, _l_param))
        return true;
    
    static bool mouse_holding = false;

    LRESULT result = 0;
    switch (_message)
    {
        case WM_SIZE:
        {
            get_app()->window_info_.width = LOWORD(_l_param);
            get_app()->window_info_.height = HIWORD(_l_param);

            int width  = get_app()->window_info_.width;
            int height = get_app()->window_info_.height;

            DLOG_TRACE("resize - width:%d height:%d", width, height);
            if (get_app()->inited_) 
                glViewport(0, 0, width, height);
        } break;
        case WM_DESTROY:
        {
            DLOG_TRACE("destroy\n");
            PostQuitMessage(0);
            // the GetMessage() will get 0 after this is invoked
            // so that we could end the message loop
        } break;
        case WM_CLOSE:
        {
            DLOG_TRACE("close\n");
            PostQuitMessage(0);
        } break;
        case WM_ACTIVATEAPP:
        {
            mouse_holding = false;
            DLOG_TRACE("activate\n");
        } break;
        case WM_MOUSEMOVE:
        {
            if (mouse_holding) {

                int wnd_width = get_app()->window_info_.width;
                int wnd_height = get_app()->window_info_.height;

                DGL::Camera* cam = &get_app()->curr_scene_->camera_;
                Image* img = &get_app()->curr_scene_->image_;
                
                glm::mat4 matrix = cam->calc_proj(wnd_width, wnd_height) * cam->calc_view();
                matrix = glm::inverse(matrix);

                struct { int x; int y; }
                    mouse_pos = { LOWORD(_l_param), HIWORD(_l_param) };

                glm::vec4 ws_pos = glm::vec4(mouse_pos.x, mouse_pos.y, 1, 1);
                
                ws_pos.x = glm::clamp(ws_pos.x, 0.0f, (float)wnd_width);
                ws_pos.y = glm::clamp(ws_pos.y, 0.0f, (float)wnd_height);

                ws_pos.x = (ws_pos.x / wnd_width) * 2.0f - 1.0f;
                ws_pos.y = ((wnd_height - ws_pos.y) / wnd_height) * 2.0f - 1.0f;

                DLOG_TRACE("wpos_x: %f - wpos_y: %f", ws_pos.x, ws_pos.y);
                // error here
                glm::vec4 cs_pos = matrix * ws_pos;
                // DLOG_TRACE("cpos_x: %f - cpos_y: %f", cs_pos.x, cs_pos.y);


                int half_width  = (int)(0.5f * img->info_.width);
                int half_height = (int)(0.5f * img->info_.height);
                
                get_app()->draw_circle(img, cs_pos.x + half_width, -cs_pos.y + half_height, 25, 0x22efcdff);
            }
        } break;
        case WM_LBUTTONDOWN:
        {
            DLOG_TRACE("mouse L button down");
            mouse_holding = true;
        } break;
        case WM_LBUTTONUP:
        {
            mouse_holding = false;
            DLOG_TRACE("mouse L button up");
        } break;
        default:
        {
            result = DefWindowProc(_window, _message, _w_param, _l_param);
            // need this to handle WM_CREATE and rerurn a non-zero value, 
            // otherwise we would failed to create a window
        } break;
    }
    return result;
}

void Application::init_window(HINSTANCE _instance, HINSTANCE _prev_instance, char* _cmd_line, int _show_code) {
    WNDCLASS wnd_class = {};
    wnd_class.style = CS_HREDRAW | CS_VREDRAW;
    wnd_class.lpfnWndProc = windows_proc;
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
    gl_info.version              = (char*)glGetString(GL_VERSION);
    gl_info.vendor               = (char*)glGetString(GL_VENDOR);
    gl_info.renderer             = (char*)glGetString(GL_RENDERER);
    gl_info.shading_lang_version = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

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
    ImGui_ImplOpenGL3_Init(nullptr);

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    // ImFont* font = io.Fonts->AddFontFromFileTTF("...", 14.0f);
    DLOG_INFO("ImGui has been initialized");
}

void Application::draw_circle(Image* _img, int _x, int _y, int _r, unsigned int _col) {
    if (_x < -_r || _x > _img->info_.width + _r || _y < -_r || _y > _img->info_.height + _r )
        return;

    // a function changing the vec2 position into an index
    auto px = [=](int _x, int _y){
        return _y * _img->info_.width + _x;
    };

    int start_y = _y - _r;

    for (int i = 0; i < glm::min(2 * _r, _img->info_.height - start_y); i++)
    {
        int line_y = start_y + i;
        if (line_y < 0) 
            continue;
        
        int scan_length = (int)(2 * glm::sqrt(_r * _r - (_r - i) * (_r - i)));

        int start_x = (int)(_x - scan_length * 0.5f);
        if (start_x < 0) {
            scan_length += start_x;
            start_x = 0;
        }

        scan_length = glm::min(scan_length, _img->info_.width - start_x - 1);

        int start = px(start_x, line_y);

        for (int j = 0; j < scan_length; j++)
        {
            char* col = (char*)&_col;
            char* pix = (char*)((int*)_img->pixels_ + start + j);
            
            int check = 0x00ffffff;

            if (*((char*)&check) == 0x00) {
                pix[0] = col[0];
                pix[1] = col[1];
                pix[2] = col[2];
                pix[3] = col[3];
            } else {
                pix[0] = col[3];
                pix[1] = col[2];
                pix[2] = col[1];
                pix[3] = col[0];
            }
        }
    }
}
