#include "Layer.h"
#include "Base/General.h"
#include "Core/Color.h"
#include "DGLCore/GLBuffer.h"
#include "DGLCore/GLEnums.h"
#include "DGLCore/GLTexture.h"
#include <stdint.h>
#include <string.h>

Layer::Layer(unsigned int _width, unsigned int _height, std::string _name, Col_RGBA _col)
:   img_(_width, _height, _col),
    dirt_region_{0}
{
    info_.name       = _name;
    info_.blend_mode = BlendMode::NORMAL;

    using namespace DGL;
    tex_.init();
    tex_.allocate(1, SizedInternalFormat::RGBA8, _width, _height);
    tex_.upload(0, 0, 0, _width, _height,
                PixFormat::RGBA, PixType::UNSIGNED_BYTE,
                img_.pixels_);

    mark_dirt({0, 0, _width, _height});
}

Layer::~Layer() {

}

void Layer::mark_dirt(Dove::IRect2D _region) {
    dirt_region_ = Dove::merge_rect(_region, dirt_region_);
}

void Layer::update_tex(bool _whole) {
    using namespace DGL;
    if (_whole) {
        tex_.upload(0, 0, 0, img_.info_.width, img_.info_.height,
                    PixFormat::RGBA, PixType::UNSIGNED_BYTE,
                    img_.pixels_);
    } else {
        // TODO: **region updating**
    }
}

// ---------------------Layer Image------------------------- //
LayerImage::LayerImage(int _width, int _height, Col_RGBA _col, bool _attach)
:   gl_attached_(false)
{
    img_ = std::make_unique<Image>(_width, _height, _col);

    if (_attach) {
        attach_gltex();
    } else {
        tex_ = nullptr;
    }
}

LayerImage::~LayerImage() {
  
}

void LayerImage::attach_gltex() {
    using namespace DGL;
    if (gl_attached_) return;

    tex_ = std::make_unique<GLTexture2D>();

    int width  = img_->info_.width;
    int height = img_->info_.height;
    tex_->init();
    tex_->param_mag_filter(DGL::TexFilter::NEAREST);
    tex_->param_min_filter(DGL::TexFilter::NEAREST);
    tex_->param_wrap_r(DGL::TexWrap::CLAMP_TO_EDGE);
    tex_->param_wrap_s(DGL::TexWrap::CLAMP_TO_EDGE);
    tex_->allocate(1, DGL::SizedInternalFormat::RGBA8, width, height);
    tex_->upload(0, 0, 0, width, height, PixFormat::RGBA, PixType::UNSIGNED_BYTE, img_->pixels_);

    gl_attached_ = true;
}

void LayerImage::update_tex(bool _whole) {
    if (!gl_attached_) return;

    using namespace DGL;
    int width  = img_->info_.width;
    int height = img_->info_.height;
    
    if (_whole) {
        tex_->upload(0, 0, 0, width, height, PixFormat::RGBA, PixType::UNSIGNED_BYTE, img_->pixels_);
    } else {
        Dove::IRect2D rect = dirt_region_;
        if (!rect.width || !rect.height) return;

        Col_RGBA* ptr = img_->pixels_ + rect.posy * img_->info_.width + rect.posx;
        for (int i = rect.posy; i < rect.posy + rect.height; i++) {
            tex_->upload(0, rect.posx, i,
                         rect.width, 1,
                         PixFormat::RGBA, PixType::UNSIGNED_BYTE, ptr);
            ptr += img_->info_.width;
        }
    }
    clear_dirt();
}

void LayerImage::mark_dirt(Dove::IRect2D _region) {
    dirt_region_ = Dove::merge_rect(_region, dirt_region_);
}

void LayerImage::clear_dirt() {
    dirt_region_ = {0};
}
