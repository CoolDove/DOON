#pragma once
#include "Core/Input.h"

namespace Tool
{
class Tool {
public:
    virtual void on_init() {}
    virtual void on_activate() {}
    virtual void on_deactivate() {}

    virtual void on_pointer_down(Input::PointerInfo _info, int _x, int _y) {}
    virtual void on_pointer_up  (Input::PointerInfo _info, int _x, int _y) {}
    virtual void on_pointer     (Input::PointerInfo _info, int _x, int _y) {}

    virtual void on_keyboard_down() {}
    virtual void on_keyboard_up() {}
    virtual void on_keyboard() {}

    virtual void on_update() {}
    virtual void on_render() {}
};
}
