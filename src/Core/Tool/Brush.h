#pragma once
#include "Tool.h"
#include <Core/Color.h>

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

    virtual void on_pointer_down(Input::PointerInfo _info, int _x, int _y);
    virtual void on_pointer_up  (Input::PointerInfo _info, int _x, int _y);
    virtual void on_pointer     (Input::PointerInfo _info, int _x, int _y);
public:
    int         size_min_;
    int         size_max_;
    Col_RGBA    col_;
private:
    void draw_circle(int _x, int _y, int _r);
private:
    Application* app_;
};
}