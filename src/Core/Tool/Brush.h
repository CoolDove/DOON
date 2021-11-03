#pragma once
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
    void resize_layer_img();

public:
    // @BrushInfo:
    int        size_max_;
    float      size_min_scale_;
    Col_RGBA   col_;

public:
    LayerImage layer_img_;
private:
    void draw_circle(int _x, int _y, int _r, const Image* _target_img);

    Application* app_;
    bool         holding_;

    Program      comp_shader_; //TODO: move this to somewhere else, maybe a Compositor class
};
}
