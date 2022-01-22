#pragma once
#include "DGLCore/GLGeoBatch.h"
#include "Tool.h"
#include <Core/Color.h>
#include <DGLCore/GLProgram.h>
#include <DGLCore/GLTexture.h>
#include <Core/Image.h>
#include <Core/Layer.h>
#include <Base/General.h>

// NOTE: brush image and texture is here,
// should them belong to Brush class?
// or they should be treated as a brush layer?
using DGL::Program;
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

    virtual void on_update();
public:
    // @BrushInfo:
    int        size_max_;
    float      size_min_scale_;
    Col_RGBA   col_;
    // NOTE: this region is used to bake brush layer into current layer, should be clear after pen released
    Dove::IRect2D painting_region_;
    DGL::GLTexture2D* brush_tex_;

    DGL::GeoBatch quad_;

private:
    Dove::IRect2D draw_circle(int _x, int _y, int _r);
    Dove::IRect2D draw_dap(int _x, int _y, int _size, float _radians);

    void clear_brush_tex(Col_RGBA color = {0x00, 0x00, 0x00, 0x00});

    GLuint fbuf_brush_;
    Application* app_;
    bool         holding_;
    DGL::Program* shader_;
};
}
