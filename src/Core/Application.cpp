#include "Application.h"
#include "DoveLog.hpp"

#include <assert.h>
#include <time.h>

#include <glad/glad.h>
#include <gl/GL.h>

#include <imgui/imgui.h>
// #include <imgui/backends/imgui_impl_sdl.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <imgui/backends/imgui_impl_opengl3.h>


using wglCreateContextAttribsARB_t = HGLRC (WINAPI *) (HDC hDC, HGLRC hshareContext, const int *attribList);

Application* Application::instance_ = nullptr;

Application::Application(HINSTANCE _instance, HINSTANCE _prev_instance, char* _cmd_line, int _show_code) 
:	cam_pos{2,2},
    cam_size(0.1f),
    window_info_{0, 0},
    gl_info{},
    inited_(false)
{
    instance_ = this;

    init_dlog();
    init_window(_instance, _prev_instance, _cmd_line, _show_code);
    inited_ = true;
    init_imgui();

    camera_ = std::make_unique<DGL::Camera>();
    camera_->set_pos(cam_pos);
    camera_->set_size(cam_size);
    
    shader_ = std::make_unique<DGL::Shader>();
    batch = new DGL::GeoBatch({{DGL::Attribute::POSITION, 3}, { DGL::Attribute::UV, 2 }});
    
    /*──────────┐
    │ load data │
    └──────────*/
    shader_->load("./res/shaders/base.vert", "./res/shaders/base.frag");
    shader_->bind();

    // images["jko"] = IMG_Load("res/textures/jko.png");
    // images["test"] = IMG_Load("res/textures/test.png");

}

Application::~Application() {

}

void Application::run() {
    // glCreateTextures(GL_TEXTURE_2D, 1, &img_id);
    // glTextureStorage2D(img_id, 1, GL_RGBA12, images["jko"]->w, images["jko"]->h);

    // glBindTextureUnit(0, img_id);
    // glUniform1i(glGetUniformLocation(shader_->get_id(), "_tex"), 0);

    // batch->add_quad(0.5f * images["jko"]->w, 0.5f * images["jko"]->h, "quad");
    // batch->add_quad(0.5f * 2048, 0.5f * 2048, "quad");
    batch->add_quad(1024, 1024, "quad");
    batch->upload();

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

    // glTextureSubImage2D(img_id, 0, 0, 0, images["jko"]->w, images["jko"]->h, GL_RGBA, GL_UNSIGNED_BYTE, images["jko"]->pixels);	
    // glBindTexture(GL_TEXTURE_2D, img_id);

    int uid_view_matrix = glGetUniformLocation(shader_->get_id(), "_view");
    int uid_proj_matrix = glGetUniformLocation(shader_->get_id(), "_proj");

    glm::mat4 view = camera_->calc_view();

    // TODO: window width and height
    int width  = get_app()->window_info_.width;
    int height = get_app()->window_info_.height;
    glm::mat4 proj = camera_->calc_proj(width, height);

    glUniformMatrix4fv(uid_view_matrix, 1, false, &view[0][0]);
    glUniformMatrix4fv(uid_proj_matrix, 1, false, &proj[0][0]);

    batch->draw_batch();

    // imgui layer
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();

    ImGui::NewFrame();
    // some imgui rendering
    ImGui::ShowDemoWindow();
    ImGui::EndFrame();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SwapBuffers(device_context_);
}

#if 0
// some space transfomation while painting
case SDL_MOUSEMOTION:
    // calc the position in canvas space:
    if (mouse_holding) {
        int width, height;
        SDL_GetWindowSize(window, &width, &height);

        glm::mat4 matrix = camera_->calc_view() * camera_->calc_proj(width, height);
        matrix = glm::inverse(matrix);

        glm::vec2* mouse_pos = &Core::Input::Mouse::position;
        glm::vec4 ws_pos = glm::vec4(mouse_pos->x, mouse_pos->y, 1, 1);
        ws_pos.x = (ws_pos.x / width) * 2.0f - 1.0f;
        ws_pos.y = -((ws_pos.y / height) * 2.0f - 1.0f);

        glm::vec4 cs_pos = matrix * ws_pos;

        int half_width  = 0.5f * images["jko"]->w;
        int half_height = 0.5f * images["jko"]->h;

        draw_circle(images["jko"], cs_pos.x + half_width, -cs_pos.y + half_height, 30, 0xffffff00);
    }
#endif

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

IMGUI_IMPL_API LRESULT  ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
// implemented in imgui_impl_win32.cpp

LRESULT CALLBACK windows_proc(HWND _window, UINT _message, WPARAM _w_param, LPARAM _l_param) {
    ImGui_ImplWin32_WndProcHandler(_window, _message, _w_param, _l_param);

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
            DLOG_TRACE("activate\n");
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
    // ImGui_ImplWin32_EnableDpiAwareness();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // ImGui_ImplSDL2_InitForOpenGL(window, &glcontext);
    ImGui_ImplWin32_Init(window_);
    ImGui_ImplOpenGL3_Init(nullptr);

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    // ImFont* font = io.Fonts->AddFontFromFileTTF("...", 14.0f);
    DLOG_INFO("ImGui has been initialized");
}

#if 0
void Application::draw_circle(SDL_Surface* _img, int _x, int _y, int _r, unsigned int _col) {
    auto px = [=](int _x, int _y){
        return _y * _img->w + _x;
    };

    int center = px(_x, _y);
 
    for (int i = 0; i < 2 * _r; i++)
    {
        int scan_length = 2 * glm::sqrt(_r * _r - (_r - i) * (_r - i));

        int start_x = _x - scan_length * 0.5f;
        int start_y = _y - _r + i;

        if (start_x > _img->w || start_x < 0 || start_y > _img->h || start_y < 0) {
            return;
        }
        scan_length = glm::min(scan_length, _img->w - start_x);
        int start = px(start_x, start_y);

        for (int j = 0; j < scan_length; j++)
        {
            int* pix = (int*)_img->pixels + start + j;
            *pix = _col;
        }
    }
}
#endif