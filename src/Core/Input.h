﻿#pragma once
#include <windows.h>

namespace Input{

using bool8         = char;
using WPARAM        = unsigned __int64;
using InputProcess  = LRESULT(CALLBACK*)(HWND _window, UINT _message, WPARAM _wparam, LPARAM _lparam);

enum class PointerButton : unsigned char {
    OTHER   = 0,
    LEFT    = 1,
    RIGHT   = 2,
    MIDDLE  = 3,
    PEN     = 4,
};

struct ButtonState {
    bool8 mouse_l;
    bool8 mouse_r;
    bool8 mouse_m;
};

struct PointerInfo {
    POINTER_PEN_INFO    pen_info;
    ButtonState         btn_state;
    PointerButton       button;
};

inline InputProcess imgui_proc = nullptr;

LRESULT CALLBACK wnd_proc(HWND _window, UINT _message, WPARAM _wparam, LPARAM _lparam);

void parse_to_btnstate_from_wparam(WPARAM _wparam, ButtonState* _state);

}