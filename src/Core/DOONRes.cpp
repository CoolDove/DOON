#include "DOONRes.h"
#include <Core/Image.h>
#include "Color.h"
#include "DGLCore/GLProgram.h"
#include "DGLCore/GLTexture.h"
#include "DoveLog.hpp"

using namespace DGL;

DOONRes::DOONRes() {
}

DOONRes::~DOONRes() {
    for (auto ite : textures_) delete ite.second;
    for (auto ite : programs_) delete ite.second;
}

// GLTexture2D
DGL::GLTexture2D* DOONRes::LoadGLTexture2D(std::string _path, std::string _name) {
    using namespace DGL;
    auto img = std::make_unique<Image>(_path.c_str(), 4);

    if (!img->inited_) {
        DLOG_ERROR("failed to load texture: %s", _path.c_str());
        return nullptr;
    }

    if (textures_.find(_name) != textures_.end()) delete textures_[_name];

    textures_[_name] = new GLTexture2D();

    int w = img->info_.width;
    int h = img->info_.height;
    
    textures_[_name]->init();
    textures_[_name]->allocate(1, SizedInternalFormat::RGBA8, w, h);
    textures_[_name]->upload(0, 0, 0, w, h, PixFormat::RGBA, PixType::UNSIGNED_BYTE, img->pixels_);

    return textures_[_name];
}
DGL::GLTexture2D* DOONRes::GetGLTexture2D(std::string _name) {
    if (textures_.find(_name) == textures_.end()) return nullptr;
    return textures_[_name];
}
int DOONRes::ReleaseGLTexture2D(std::string _name) {
    if (textures_.find(_name) == textures_.end()) return 0;
    delete textures_[_name];
    textures_.erase(_name);
    return 1;
}

// Program(Shader)
DGL::Program *DOONRes::LoadShader(std::string _vertpath, std::string _fragpath, std::string _name) {
    Shader vert(_vertpath, ShaderType::VERTEX_SHADER);
    Shader frag(_fragpath, ShaderType::FRAGMENT_SHADER);

    if (!vert.good() || !frag.good()) return nullptr;

    if (programs_.find(_name) != programs_.end()) delete programs_[_name];

    auto shader = new Program();
    programs_[_name] = shader;
    shader->link(2, &vert, &frag);

    return shader;
}
DGL::Program *DOONRes::GetShader(std::string _name) {
    if (programs_.find(_name) == programs_.end()) return nullptr;
    return programs_[_name];
}
int DOONRes::ReleaseGLShader(std::string _name) {
    // TODO:
    return 1;
}
