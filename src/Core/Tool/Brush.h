#pragma once
#include "Tool.h"
#include <Core/Color.h>
#include <DGLCore/GLShader.h>
#include <DGLCore/GLTexture.h>
#include <Core/Image.h>

using namespace DGL;
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

    virtual void on_update();

    virtual void on_pointer_down(Input::PointerInfo _info, int _x, int _y);
    virtual void on_pointer_up  (Input::PointerInfo _info, int _x, int _y);
    virtual void on_pointer     (Input::PointerInfo _info, int _x, int _y);

public:
    void resize_image_and_tex();

    Image*       get_image() { return &image_; };
    GLTexture2D* get_tex() { return &tex_; };
   
public:
    int         size_max_;
    float       size_min_scale_;
    Col_RGBA    col_;
private:
    void draw_circle(int _x, int _y, int _r, const Image* _target_img);
private:
    Application* app_;
    bool         holding_;

    Image        image_;
    GLTexture2D  tex_;

private:
    // @temp: doesn't belong here, should be removed someday
    DGL::Shader* composition_shader_;    
};
}
