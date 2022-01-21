#include "DOONRes.h"
#include <Core/Image.h>
#include "Color.h"
#include "DoveLog.hpp"

DOONRes::DOONRes() {
}

DOONRes::~DOONRes() {
    for (auto ite : textures_) delete ite.second;
}

int DOONRes::LoadGLTexture2D(std::string _path, std::string _name) {
    using namespace DGL;
    auto img = std::make_unique<Image>(_path.c_str(), 4);

    if (!img->inited_) {
        DLOG_ERROR("failed to load texture: %s", _path.c_str());
        return 0;
    }

    if (textures_.find(_name) != textures_.end()) delete textures_[_name];

    textures_[_name] = new GLTexture2D();

    int w = img->info_.width;
    int h = img->info_.height;
    
    textures_[_name]->init();
    textures_[_name]->allocate(1, SizedInternalFormat::RGBA8, w, h);
    textures_[_name]->upload(0, 0, 0, w, h, PixFormat::RGBA, PixType::UNSIGNED_BYTE, img->pixels_);

    return 1;
}

DGL::GLTexture2D* DOONRes::GetGLTexture2D(std::string _name) {
    return textures_[_name];
}

int DOONRes::ReleaseGLTexture2D(std::string _name) {
    if (textures_.find(_name) == textures_.end()) return 0;
    delete textures_[_name];
    textures_.erase(_name);
    return 1;
}
