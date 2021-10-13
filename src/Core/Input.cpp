﻿#include "Input.h"
#include <Core/Application.h>
#include <DoveLog.hpp>

namespace Input {
LRESULT CALLBACK wnd_proc(HWND _window, UINT _message, WPARAM _wparam, LPARAM _lparam) {
    if (imgui_proc && imgui_proc(_window, _message, _wparam, _lparam)) 
        return true;
    
    
    static POINTER_PEN_INFO pen_info{};

    Application* app = get_app();

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
            app->window_info_.posx = LOWORD(_lparam);
            app->window_info_.posy = HIWORD(_lparam);
            DLOG_TRACE("move pos(%d, %d)", app->window_info_.posx, app->window_info_.posy);
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

            pen_info = {0};
            PointerInfo info = {0};
            info.pen_info = pen_info;
            info.button = Input::PointerButton::LEFT;
            Input::parse_to_btnstate_from_wparam(_wparam, &info.btn_state);
            app->curr_tool_->on_pointer_down(info, LOWORD(_lparam), HIWORD(_lparam));
        } break;
        case WM_LBUTTONUP:
        {
            if (!app->curr_tool_) break;

            pen_info = {0};
            PointerInfo info = {0};
            info.pen_info = pen_info;
            info.button = Input::PointerButton::LEFT;
            Input::parse_to_btnstate_from_wparam(_wparam, &info.btn_state);
            app->curr_tool_->on_pointer_up(info, LOWORD(_lparam), HIWORD(_lparam));
        } break;
        case WM_RBUTTONDOWN:
        {
            if (!app->curr_tool_) break;

            pen_info = {0};
            PointerInfo info = {0};
            info.pen_info = pen_info;
            info.button = Input::PointerButton::RIGHT;
            Input::parse_to_btnstate_from_wparam(_wparam, &info.btn_state);
            app->curr_tool_->on_pointer_down(info, LOWORD(_lparam), HIWORD(_lparam));
        } break;
        case WM_RBUTTONUP:
        {
            if (!app->curr_tool_) break;

            pen_info = {0};
            PointerInfo info = {0};
            info.pen_info = pen_info;
            info.button = Input::PointerButton::RIGHT;
            Input::parse_to_btnstate_from_wparam(_wparam, &info.btn_state);
            app->curr_tool_->on_pointer_up(info, LOWORD(_lparam), HIWORD(_lparam));
        } break;
        case WM_MBUTTONDOWN:
        {
            if (!app->curr_tool_) break;

            pen_info = {0};
            PointerInfo info = {0};
            info.pen_info = pen_info;
            info.button = Input::PointerButton::MIDDLE;
            Input::parse_to_btnstate_from_wparam(_wparam, &info.btn_state);
            app->curr_tool_->on_pointer_down(info, LOWORD(_lparam), HIWORD(_lparam));
        } break;
        case WM_MBUTTONUP:
        {
            if (!app->curr_tool_) break;

            pen_info = {0};
            PointerInfo info = {0};
            info.pen_info = pen_info;
            info.button = Input::PointerButton::MIDDLE;
            Input::parse_to_btnstate_from_wparam(_wparam, &info.btn_state);
            app->curr_tool_->on_pointer_up(info, LOWORD(_lparam), HIWORD(_lparam));
        } break;
        case WM_MOUSEMOVE:
        {
            if (!app->curr_tool_) break;

            pen_info = {0};
            PointerInfo info = {0};
            info.pen_info = pen_info;
            info.button = Input::PointerButton::OTHER;
            Input::parse_to_btnstate_from_wparam(_wparam, &info.btn_state);
            app->curr_tool_->on_pointer(info, LOWORD(_lparam), HIWORD(_lparam));
        } break;
        case WM_POINTERDOWN:
        {
            if (!app->curr_tool_) break;

            PointerInfo info = {0};
            GetPointerPenInfo(GET_POINTERID_WPARAM(_wparam), &pen_info);
            info.pen_info = pen_info;
            info.button = Input::PointerButton::PEN;
            info.btn_state = {0};
            app->curr_tool_->on_pointer_down(info, LOWORD(_lparam), HIWORD(_lparam));
        } break;
        case WM_POINTERUPDATE:
        {
            if (!app->curr_tool_) break;

            PointerInfo info = {0};
            GetPointerPenInfo(GET_POINTERID_WPARAM(_wparam), &pen_info);
            info.pen_info = pen_info;
            info.button = Input::PointerButton::PEN;
            info.btn_state = {0};
            app->curr_tool_->on_pointer(info, LOWORD(_lparam) - app->window_info_.posx, HIWORD(_lparam) - app->window_info_.posy);
        } break;
        case WM_POINTERUP:
        {
            if (!app->curr_tool_) break;

            PointerInfo info = {0};
            GetPointerPenInfo(GET_POINTERID_WPARAM(_wparam), &pen_info);
            info.pen_info = pen_info;
            info.button = Input::PointerButton::PEN;
            info.btn_state = {0};
            app->curr_tool_->on_pointer_up(info, LOWORD(_lparam), HIWORD(_lparam));
        } break;
        default:
        {
            result = DefWindowProc(_window, _message, _wparam, _lparam);
        } break;
    }
    return result;
}
void parse_to_btnstate_from_wparam(WPARAM _wparam, ButtonState* _state) {
    _state->mouse_l = _wparam & MK_LBUTTON;
    _state->mouse_r = _wparam & MK_RBUTTON;
    _state->mouse_m = _wparam & MK_MBUTTON;
}

}