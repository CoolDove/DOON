#include "DOONRes.h"
#include <Core/Image.h>
#include "Color.h"
#include "DGLCore/GLProgram.h"
#include "DGLCore/GLTexture.h"
#include "DoveLog.hpp"
#include "io.h"

using namespace DGL;

DOONRes::DOONRes() {
}

DOONRes::~DOONRes() {
    for (auto ite : textures_) delete ite.second;
    for (auto ite : programs_) delete ite.second;
}

void DOONRes::SetResourcePath(const std::string path) {
    res_path_ = path;
}

void DOONRes::LoadResourcesPath() {
    find_textures();
    find_shaders();
}

void DOONRes::find_textures() {
    std::string path = res_path_ + "/textures/";
    std::string search_string = path + "*.png";
    intptr_t handle = 0;
    _finddata_t info;

    handle = _findfirst(search_string.c_str(), &info);
    if (-1 == handle) return;

    while (true) {
        std::string path_name = path + info.name;
        std::string texture_name = info.name;
        size_t pos = texture_name.find_last_of(".");
        texture_name = texture_name.substr(0, pos);
        DLOG_DEBUG("load texture [%s] from %s", texture_name.c_str(), path_name.c_str());
        LoadGLTexture2D(path_name.c_str(), texture_name);

        if (_findnext(handle, &info)) break;
    }
    _findclose(handle);
}

void DOONRes::find_shaders() {
    std::string path = res_path_ + "/shaders/";
    std::string search_vert = path + "*.vert";
    std::string search_frag = path + "*.frag";

    intptr_t vert_handle = 0;
    _finddata_t info;

    vert_handle = _findfirst(search_vert.c_str(), &info);

    if (-1 == vert_handle) return;
    
    while (true) {
        std::string shader_name = info.name;
        size_t pos = shader_name.find_last_of(".");
        shader_name = shader_name.substr(0, pos);

        std::string name_with_path_vert = path + info.name;
        std::string name_with_path_frag = path + shader_name + ".frag";
        auto output = LoadShader(name_with_path_vert, name_with_path_frag, shader_name);
        if (output) DLOG_DEBUG("load shader [%s] from %s, %s", shader_name.c_str(), name_with_path_vert.c_str(), name_with_path_frag.c_str());

        if (_findnext(vert_handle, &info)) break;
    }
  
    _findclose(vert_handle);
}

// GLTexture2D
DGL::GLTexture2D* DOONRes::LoadGLTexture2D(const std::string& _path, const std::string& _name) {
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
DGL::GLTexture2D* DOONRes::GetGLTexture2D(const std::string& _name) {
    if (textures_.find(_name) == textures_.end()) return nullptr;
    return textures_[_name];
}
int DOONRes::ReleaseGLTexture2D(const std::string& _name) {
    if (textures_.find(_name) == textures_.end()) return 0;
    delete textures_[_name];
    textures_.erase(_name);
    return 1;
}

// Program(Shader)
DGL::Program *DOONRes::LoadShader(const std::string& _vertpath, const std::string& _fragpath, const std::string& _name) {
    Shader vert(_vertpath, ShaderType::VERTEX_SHADER);
    Shader frag(_fragpath, ShaderType::FRAGMENT_SHADER);

    if (!vert.good() || !frag.good()) return nullptr;

    if (programs_.find(_name) != programs_.end()) delete programs_[_name];

    auto shader = new Program();
    programs_[_name] = shader;
    shader->link(2, &vert, &frag);

    return shader;
}
DGL::Program *DOONRes::GetShader(const std::string& _name) {
    if (programs_.find(_name) == programs_.end()) return nullptr;
    return programs_[_name];
}
int DOONRes::ReleaseGLShader(const std::string& _name) {
    // TODO:
    return 1;
}
