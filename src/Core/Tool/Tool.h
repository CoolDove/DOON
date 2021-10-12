#pragma once
#include "Core/Input.h"

namespace Tool
{
class Tool {
public:
    virtual void on_init() = 0;
    virtual void on_activate() = 0;
    virtual void on_deactivate() = 0;

    virtual void on_mouse_down  (Input::MouseInfo _info,
                                 Input::MouseButton _button,
                                 int _x, int _y) = 0;

    virtual void on_mouse_up    (Input::MouseInfo _info,
                                 Input::MouseButton _button,
                                 int _x, int _y) = 0;
    virtual void on_mouse_move  (Input::MouseInfo _info, int _x, int _y) = 0;

    virtual void on_keyboard_down() = 0;
    virtual void on_keyboard_up() = 0;
    virtual void on_keyboard_move() = 0;

    virtual void on_pen_down() = 0;
    virtual void on_pen_up() = 0;
    virtual void on_pen_move() = 0;
};
}