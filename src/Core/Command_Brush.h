#pragma once
#include <Core/Application.h>
#include <Core/Renderer.h>
#include <Core/Command.h>
#include <DGLCore/GLTexture.h>
#include <DoveLog.hpp>
#include <Base/General.h>

class BrushCommand : public Command
{
public:
    /* NOTE:
    *  create this command before blit brush layer, it will create stash texture
    *  from current layer while constructing.
    */
    BrushCommand(Dove::IRect2D _rect, Application* p_app) {
        using namespace DGL;
        using namespace Dove;
        auto* renderer = p_app->renderer_.get();
        auto* scn = p_app->curr_scene_;
        target_ = scn->get_curr_layer()->tex_.get();// current layer texture

        auto* brush_tex = &scn->brush_layer_;

        stash_tex_ = new GLTexture2D();
        stash_tex_->init();
        stash_tex_->allocate(1, SizedInternalFormat::RGBA8, _rect.width, _rect.height);

        rect_ = _rect;

        IRect2D rect_src = rect_;
        IRect2D rect_dst = rect_src;
        rect_dst.position = {0};
        Renderer::blit(target_, stash_tex_, rect_src, rect_dst);
    }
    
    ~BrushCommand() {
        delete stash_tex_;
    }

public:
    Dove::IRect2D rect_;
    DGL::GLTexture2D* stash_tex_;
    DGL::GLTexture2D* target_;

    void on_undo() override {
        using namespace DGL;
        using namespace Dove;

        GLTexture2D temp;
        temp.init();
        temp.allocate(1, SizedInternalFormat::RGBA8, rect_.width, rect_.height);
        IRect2D rect_stash = rect_;
        rect_stash.position = {0};
        
        Renderer::blit(stash_tex_, &temp, rect_stash, rect_stash);
        Renderer::blit(target_, stash_tex_, rect_, rect_stash);
        Renderer::blit(&temp, target_, rect_stash, rect_);
    }

    void on_redo() override {
        using namespace DGL;
        using namespace Dove;

        GLTexture2D temp;
        temp.init();
        temp.allocate(1, SizedInternalFormat::RGBA8, rect_.width, rect_.height);
        IRect2D rect_stash = rect_;
        rect_stash.position = {0};
        
        Renderer::blit(stash_tex_, &temp, rect_stash, rect_stash);
        Renderer::blit(target_, stash_tex_, rect_, rect_stash);
        Renderer::blit(&temp, target_, rect_stash, rect_);
    }
};
