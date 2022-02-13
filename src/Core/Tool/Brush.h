#pragma once
#include "DGLCore/GLGeoBatch.h"
#include "Tool.h"
#include <Core/Color.h>
#include <DGLCore/GLProgram.h>
#include <DGLCore/GLTexture.h>
#include <DGLCore/GLFramebuffer.h>
#include <Core/Image.h>
#include <Core/Layer.h>
#include <Base/General.h>

using DGL::Program;
class Application;
namespace Tool
{

struct BrushDap
{
    Dove::IVector2D position;
    float radians;
    float radius;
};

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
    int      size_max_;
    float    size_min_scale_;
    float    distance_;

    Col_RGBA col_;

    Dove::IRect2D painting_region_;
    DGL::GLTexture2D* brush_tex_;

    DGL::GeoBatch quad_;

private: // blend things
    DGL::GLTexture2D* blend_tex_a_ = nullptr;
    DGL::GLTexture2D* blend_tex_b_ = nullptr;
    DGL::GLTexture2D* blend_attaching_ = nullptr;
    DGL::GLTexture2D* blend_test = nullptr;
    DGL::GLFramebuffer* blend_framebuf_ = nullptr;

    DGL::GLTexture2D* blend_attaching_tex() const { return blend_attaching_; }
    DGL::GLTexture2D* blend_other_tex() const {
        if (blend_attaching_ == nullptr) return nullptr;
        else if (blend_attaching_ == blend_tex_a_) {
            return blend_tex_b_;
        } else if (blend_attaching_ == blend_tex_b_) {
            return blend_tex_a_;
        }
        return nullptr;
    }

    void create_blend_assets();
    void release_blend_assets();
    void switch_attaching_texture();

private:
    std::list<BrushDap> mpoints_;// store the mouse point info
    std::list<BrushDap> daps_;// store the interpolated points since last mouse point pushed
    Dove::IVector2D last_dap_pos_;
    float last_brush_size_;

    Dove::IVector2D last_mouse_pos_;

    float calculate_brush_size(const Input::PointerInfo* _info);
    void draw_daps();// draw every dap in the daps and clear the daps
    void generate_daps(Dove::IVector2D mouse_pos_canvas_space, const Input::PointerInfo* brush_info);
    void worldpos_to_canvaspos(int wx, int wy, int* cx, int* cy);

    void flush_data();// bake brush layer to current layer and push brush command, then clear the brush layer
    
private:
    Dove::IRect2D draw_circle(int _x, int _y, int _r);

    void clear_brush_tex(Col_RGBA color = {0x00, 0x00, 0x00, 0x00});

    GLuint fbuf_brush_;
    Application* app_;
    bool         holding_;
    DGL::Program* shader_;
};
}
