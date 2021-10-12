#pragma once
#include "Tool.h"

class Application;

namespace Tool
{
class Brush : public Tool
{
public:
    Brush(Application* _app);
    ~Brush();
public:
    virtual void on_init();
    virtual void on_activate();
    virtual void on_deactivate();

    virtual void on_mouse_down  (Input::MouseInfo _info,
                                 Input::MouseButton _button,
                                 int _x, int _y);

    virtual void on_mouse_up    (Input::MouseInfo _info,
                                 Input::MouseButton _button,
                                 int _x, int _y);

    virtual void on_mouse_move  (Input::MouseInfo _info, int _x, int _y);

    virtual void on_keyboard_down();
    virtual void on_keyboard_up();
    virtual void on_keyboard_move();

    virtual void on_pen_down();
    virtual void on_pen_up();
    virtual void on_pen_move();
private:
    Application* app_;

};
}