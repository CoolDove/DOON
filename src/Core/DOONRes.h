#pragma once
#include <unordered_map>
#include <string>
#include <DGLCore/GLTexture.h>
#include <DGLCore/GLProgram.h>

class DOONRes
{
public:
    DOONRes();
    ~DOONRes();

    int LoadGLTexture2D(std::string _path, std::string _name);
    DGL::GLTexture2D* GetGLTexture2D(std::string _name);
    int ReleaseGLTexture2D(std::string _name);

private:
    std::unordered_map<std::string, DGL::GLTexture2D*> textures_;
    std::unordered_map<std::string, DGL::Program*> programs_;
};
