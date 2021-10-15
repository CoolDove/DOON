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

    virtual void on_pointer_down(Input::PointerInfo _info, int _x, int _y);
    virtual void on_pointer_up  (Input::PointerInfo _info, int _x, int _y);
    virtual void on_pointer     (Input::PointerInfo _info, int _x, int _y);
public:
    float col_[4];
private:
    void draw_circle(int _x, int _y, int _r, unsigned int _col);
private:
    Application* app_;
};
}