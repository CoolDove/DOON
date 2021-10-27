﻿#include "Input.h"
#include <Core/Application.h>
#include <DoveLog.hpp>
#include <imgui/imgui.h>

#define IMGUI_MONOPOLY_INPUT 0x0010
namespace Input {
InputProcess imgui_proc = nullptr;
InputContext input_context = {0};

// FIXME: pen did not release after leaving the client region
// FIXME: when mouse position is outside the client region, imgui cannot be clicked by pen

// check if mouse is in region for tools
bool is_mouse_position_valid(Application* _app, int _x, int _y) {
    return !(_x >= _app->window_info_.width || _x <= 0 ||
             _y >= _app->window_info_.height|| _y <= 0);
}

// NOTE: |[Mouse Message Rules For Tools]|
// when mouse position is in tool region, clicking will set the state in 
// input_context no matter whether imgui is capturing mouse.
// when you release mouse(imgui capturing or not), or move out tool region,
// a pointer_up() is called, and the corresponding value in input_context will be set
// [ button down ] should be blocked by imgui, but [ button up ] should go through
LRESULT CALLBACK wnd_proc(HWND _window, UINT _message, WPARAM _wparam, LPARAM _lparam) {
    if (imgui_proc) {
        LRESULT rst = imgui_proc(_window, _message, _wparam, _lparam);
        if (rst == IMGUI_MONOPOLY_INPUT) {
            return 1;
        }
    }
    
    static POINTER_PEN_INFO pen_info{};

    Application* app = Application::instance_;

    LRESULT result = 0;
    switch (_message)
    {
        case WM_SIZE:
        {
            app->window_info_.width = LOWORD(_lparam);
            app->window_info_.height = HIWORD(_lparam);

            int width  = app->window_info_.width;
            int height = app->window_info_.height;

            if (app->inited_) 
                glViewport(0, 0, width, height);

        } break;
        case WM_MOVE:
        {
            // NOTE: this is window move, not mouse move
            app->window_info_.posx = LOWORD(_lparam);
            app->window_info_.posy = HIWORD(_lparam);
        } break;
        case WM_DESTROY:
        {
            PostQuitMessage(0);
        } break;
        case WM_CLOSE:
        {
            PostQuitMessage(0);
        } break;
        case WM_ACTIVATEAPP:
        {
            DLOG_TRACE("activate\n");
        } break;
        case WM_LBUTTONDOWN:
        {
            if (!app->curr_tool_) break;

            int mx = LOWORD(_lparam);
            int my = HIWORD(_lparam);

            if (is_mouse_position_valid(app, mx, my)) {
                input_context.mouse_down_l = true;

                if (!ImGui::GetIO().WantCaptureMouse) {
                    pen_info = {0};
                    pen_info.pressure = 1;
                    PointerInfo info = {0};
                    info.pen_info = pen_info;
                    info.button = Input::PointerButton::LEFT;
                    Input::get_btnstate_from_wparam(_wparam, &info.btn_state);
                    app->curr_tool_->on_pointer_down(info, mx, my);
                }
            }
        } break;
        case WM_LBUTTONUP:
        {
            if (!app->curr_tool_) break;

            int mx = LOWORD(_lparam);
            int my = HIWORD(_lparam);
            if (is_mouse_position_valid(app, mx, my) || input_context.mouse_down_l) {
                input_context.mouse_down_l = false;

                if (!ImGui::GetIO().WantCaptureMouse) {
                    pen_info = {0};
                    PointerInfo info = {0};
                    info.pen_info = pen_info;
                    info.button = Input::PointerButton::LEFT;
                    Input::get_btnstate_from_wparam(_wparam, &info.btn_state);
                    app->curr_tool_->on_pointer_up(info, mx, my);
                }
            }
        } break;
        case WM_RBUTTONDOWN:
        {
            if (!app->curr_tool_) break;

            int mx = LOWORD(_lparam);
            int my = HIWORD(_lparam);
            if (is_mouse_position_valid(app, mx, my)) {
                input_context.mouse_down_r = true;
                if (!ImGui::GetIO().WantCaptureMouse) {

                    pen_info = {0};
                    PointerInfo info = {0};
                    info.pen_info = pen_info;
                    info.button = Input::PointerButton::RIGHT;
                    Input::get_btnstate_from_wparam(_wparam, &info.btn_state);
                    app->curr_tool_->on_pointer_down(info, mx, my);
                }
            }
        } break;
        case WM_RBUTTONUP:
        {
            if (!app->curr_tool_) break;

            if (!ImGui::GetIO().WantCaptureMouse) {
                pen_info = {0};
                PointerInfo info = {0};
                info.pen_info = pen_info;
                info.button = Input::PointerButton::RIGHT;
                Input::get_btnstate_from_wparam(_wparam, &info.btn_state);
                app->curr_tool_->on_pointer_up(info, LOWORD(_lparam), HIWORD(_lparam));
            }

            input_context.mouse_down_r = false;
        } break;
        case WM_MBUTTONDOWN:
        {
            if (!app->curr_tool_) break;

            if (!ImGui::GetIO().WantCaptureMouse) {
                pen_info = {0};
                PointerInfo info = {0};
                info.pen_info = pen_info;
                info.button = Input::PointerButton::MIDDLE;
                Input::get_btnstate_from_wparam(_wparam, &info.btn_state);
                app->curr_tool_->on_pointer_down(info, LOWORD(_lparam), HIWORD(_lparam));
            }
            
            input_context.mouse_down_m = true;
        } break;
        case WM_MBUTTONUP:
        {
            if (!app->curr_tool_) break;

            if (!ImGui::GetIO().WantCaptureMouse) {
                pen_info = {0};
                PointerInfo info = {0};
                info.pen_info = pen_info;
                info.button = Input::PointerButton::MIDDLE;
                Input::get_btnstate_from_wparam(_wparam, &info.btn_state);
                app->curr_tool_->on_pointer_up(info, LOWORD(_lparam), HIWORD(_lparam));
            }

            input_context.mouse_down_m = false;
        }
        case WM_MOUSEMOVE:
        {
            if (!app->curr_tool_) break;

            int mx = LOWORD(_lparam);
            int my = HIWORD(_lparam);

            // valid, invoke on_pointer
            if (is_mouse_position_valid(app, mx, my)) {
                if (!ImGui::GetIO().WantCaptureMouse) {
                    pen_info = {0};
                    PointerInfo info = {0};
                    info.pen_info = pen_info;
                    info.button = Input::PointerButton::OTHER;
                    Input::get_btnstate_from_wparam(_wparam, &info.btn_state);
                    
                    app->curr_tool_->on_pointer(info, LOWORD(_lparam), HIWORD(_lparam));
                }
            } else {
                // valid, invoke pointer_up
                if (!app->curr_tool_) break;

                int mx = LOWORD(_lparam);
                int my = HIWORD(_lparam);
                // mouse move outside the tool region
                // if any mouse button is holding, corresponding pointer_up() should be called
                if (input_context.mouse_down_l) {
                    input_context.mouse_down_l = false;
                    pen_info = {0};
                    PointerInfo info = {0};
                    info.pen_info = pen_info;
                    info.button = Input::PointerButton::LEFT;
                    Input::get_btnstate_from_wparam(_wparam, &info.btn_state);
                    info.btn_state.mouse_l = false;
                    app->curr_tool_->on_pointer_up(info, mx, my);
                }
                if (input_context.mouse_down_r) {
                    input_context.mouse_down_r = false;
                    pen_info = {0};
                    PointerInfo info = {0};
                    info.pen_info = pen_info;
                    info.button = Input::PointerButton::LEFT;
                    Input::get_btnstate_from_wparam(_wparam, &info.btn_state);
                    info.btn_state.mouse_r = false;
                    app->curr_tool_->on_pointer_up(info, mx, my);
                }
                if (input_context.mouse_down_m) {
                    input_context.mouse_down_m = false;
                    pen_info = {0};
                    PointerInfo info = {0};
                    info.pen_info = pen_info;
                    info.button = Input::PointerButton::LEFT;
                    Input::get_btnstate_from_wparam(_wparam, &info.btn_state);
                    info.btn_state.mouse_m = false;
                    app->curr_tool_->on_pointer_up(info, mx, my);
                }
            }

            input_context.mouse_pos.x = LOWORD(_lparam);
            input_context.mouse_pos.y = HIWORD(_lparam);
        } break;
        case WM_POINTERDOWN:
        {
            if (!app->curr_tool_) break;

            if (!ImGui::GetIO().WantCaptureMouse) {
                PointerInfo info = {0};
                GetPointerPenInfo(GET_POINTERID_WPARAM(_wparam), &pen_info);
                info.pen_info = pen_info;
                info.button = Input::PointerButton::PEN;
                info.btn_state = {0};
                app->curr_tool_->on_pointer_down(info, LOWORD(_lparam), HIWORD(_lparam));
            }

            input_context.pen_down = true;

        } break;
        case WM_POINTERUPDATE:
        {
            if (!app->curr_tool_) break;

            if (!ImGui::GetIO().WantCaptureMouse) {
                PointerInfo info = {0};
                GetPointerPenInfo(GET_POINTERID_WPARAM(_wparam), &pen_info);
                info.pen_info = pen_info;
                info.button = Input::PointerButton::PEN;
                info.btn_state = {0};
                app->curr_tool_->on_pointer(info, LOWORD(_lparam) - app->window_info_.posx, HIWORD(_lparam) - app->window_info_.posy);
            }

            input_context.mouse_pos.x = LOWORD(_lparam) - app->window_info_.posx;
            input_context.mouse_pos.y = HIWORD(_lparam) - app->window_info_.posy;

        } break;
        case WM_POINTERUP:
        {
            DLOG_TRACE("receive pointer up");
            if (!app->curr_tool_) break;

            if (!ImGui::GetIO().WantCaptureMouse) {
                PointerInfo info = {0};
                GetPointerPenInfo(GET_POINTERID_WPARAM(_wparam), &pen_info);
                info.pen_info = pen_info;
                info.button = Input::PointerButton::PEN;
                info.btn_state = {0};
                app->curr_tool_->on_pointer_up(info, LOWORD(_lparam), HIWORD(_lparam));
            }

            input_context.pen_down = false;
        } break;
        case WM_MOUSELEAVE:
        {
            DLOG_TRACE("mouse leaved");
        } break;
        default:
        {
            result = DefWindowProc(_window, _message, _wparam, _lparam);
        } break;
    }

    // check mouse leaved
    if (GetCapture() == _window &&
       (input_context.mouse_pos.x > app->window_info_.width||
        input_context.mouse_pos.y > app->window_info_.height))
    {
        if (input_context.pen_down) 
            input_context.pen_down = false;
    }

    if (app->inited_) {
        ImGuiIO& io = ImGui::GetIO();
        io.MousePos.x = (float)input_context.mouse_pos.x;
        io.MousePos.y = (float)input_context.mouse_pos.y;
        io.MouseDown[0] = input_context.mouse_down_l | input_context.pen_down;
        io.MouseDown[1] = input_context.mouse_down_r;
        io.MouseDown[3] = input_context.mouse_down_m;
    }
    return result;
}

void get_btnstate_from_wparam(WPARAM _wparam, ButtonState* _state) {
    _state->mouse_l = _wparam & MK_LBUTTON;
    _state->mouse_r = _wparam & MK_RBUTTON;
    _state->mouse_m = _wparam & MK_MBUTTON;
}

}