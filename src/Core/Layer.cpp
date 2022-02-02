﻿#include "Layer.h"
#include "Base/General.h"
#include "Core/Color.h"
#include "DGLCore/GLBuffer.h"
#include "DGLCore/GLEnums.h"
#include "DGLCore/GLTexture.h"
#include <stdint.h>
#include <string.h>

Layer::Layer(const std::string& path) {
    using namespace DGL;
    Image img(path.c_str(), 4);

    assert(img.pixels_);

    info_.name = path;
    info_.blend_mode = BlendMode::NORMAL;
    info_.width = img.info_.width;
    info_.height = img.info_.height;

    tex_ = std::make_unique<GLTexture2D>();
    tex_->init();
    tex_->param_mag_filter(TexFilter::NEAREST);
    tex_->param_min_filter(TexFilter::NEAREST);
    tex_->param_wrap_r(TexWrap::CLAMP_TO_EDGE);
    tex_->param_wrap_s(TexWrap::CLAMP_TO_EDGE);

    tex_->allocate(1, SizedInternalFormat::RGBA8, info_.width, info_.height);

    pixels_ = (Col_RGBA*)malloc(data_size());
    memcpy(pixels_, img.pixels_, data_size());

    update_tex();
}

Layer::Layer(unsigned int _width, unsigned int _height, std::string _name, Col_RGBA _col) {
    using namespace DGL;
    info_.name       = _name;
    info_.blend_mode = BlendMode::NORMAL;

    Image img_(_width, _height, _col);
    tex_ = std::make_unique<GLTexture2D>();

    info_.width = _width;
    info_.height = _height;

    tex_->init();
    tex_->param_mag_filter(TexFilter::NEAREST);
    tex_->param_min_filter(TexFilter::NEAREST);
    tex_->param_wrap_r(TexWrap::CLAMP_TO_EDGE);
    tex_->param_wrap_s(TexWrap::CLAMP_TO_EDGE);

    tex_->allocate(1, SizedInternalFormat::RGBA8, _width, _height);
    tex_->upload(0, 0, 0, _width, _height,
                PixFormat::RGBA, PixType::UNSIGNED_BYTE,
                img_.pixels_);

    pixels_ = (Col_RGBA*)malloc(sizeof(Col_RGBA) * info_.width * info_.height);
}

Layer::~Layer() {
    if (pixels_) {
        mem_release();
    }
}

void Layer::update_tex() {
    using namespace DGL;
    tex_->upload(
        0, 0, 0, info_.width, info_.height,
        PixFormat::RGBA, PixType::UNSIGNED_BYTE,
        pixels_);
}

void Layer::mem_fetch() {

}

void Layer::mem_release() {
    if (pixels_) {
        free(pixels_);
        pixels_ = nullptr;
    }
}
