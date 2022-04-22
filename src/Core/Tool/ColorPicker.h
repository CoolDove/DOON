#pragma once
#include "Tool.h"
#include <Core/Color.h>

class Application;

namespace Tool
{
class ColorPicker : public Tool
{
public:
    ColorPicker();
    ~ColorPicker();
public:
    virtual void on_init();
    virtual void on_activate();
    virtual void on_deactivate();

    virtual void on_pointer_down(Input::PointerInfo _info, int _x, int _y);
    virtual void on_pointer_up  (Input::PointerInfo _info, int _x, int _y);
    virtual void on_pointer     (Input::PointerInfo _info, int _x, int _y);

    virtual void on_update();

private:
    Col_RGBA* buffer;
};
};
