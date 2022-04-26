#pragma once
#include <unordered_map>
#include <string>
#include <Core/Scene.h>
#include <DGLCore/GLTexture.h>
#include <DGLCore/GLProgram.h>

class DOONRes
{
public:
    DOONRes();
    ~DOONRes();

    std::string res_path_;
    void SetResourcePath(const std::string path);

    void LoadResourcesPath();

    DGL::GLTexture2D* LoadGLTexture2D(const std::string& _path, const std::string& _name);
    DGL::GLTexture2D* GetGLTexture2D(const std::string& _name);
    int ReleaseGLTexture2D(const std::string& _name);

    DGL::Program* LoadShader(const std::string& _vertpath, const std::string& _fragpath, const std::string& _name);
    DGL::Program* GetShader(const std::string& _name);
    int ReleaseGLShader(const std::string& _name);
private:
    void find_textures();
    void find_shaders();

private:
    std::unordered_map<std::string, DGL::GLTexture2D*> textures_;
    std::unordered_map<std::string, DGL::Program*> programs_;
};
