﻿#pragma once
#include <windows.h>
#include <stdint.h>
#include <Base/BitMaskEnum.h>
#include <Base/Keys.h>

namespace Input{
using namespace Dove;

using bool8         = uint8_t;
using WPARAM        = unsigned __int64;
using InputProcess  = LRESULT(CALLBACK*)(HWND _window, UINT _message, WPARAM _wparam, LPARAM _lparam);
using KeyCode       = unsigned long long;

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

struct InputContext {
    struct {
        int x;
        int y;
    } mouse_pos;
    bool8  mouse_down_l;
    bool8  mouse_down_r;
    bool8  mouse_down_m;
    bool8  pen_down;
    Dove::ModKey mod_key;
};

extern InputProcess imgui_proc;

extern InputContext input_context;

LRESULT CALLBACK wnd_proc(HWND _window, UINT _message, WPARAM _wparam, LPARAM _lparam);

void get_btnstate_from_wparam(WPARAM _wparam, ButtonState* _state);

}
