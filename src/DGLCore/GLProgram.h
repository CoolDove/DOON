#pragma once

#include "GLShader.h"

#include <glad/glad.h>
#include <string>
#include <stdint.h>
#include <vector>

namespace DGL
{
class Program {
public:
    Program();
    ~Program();

    void init();
    void link(std::vector<Shader*> _shaders);
    void bind();

public:
    uint32_t get_glid() const { return id_; };
    bool     get_inited() const { return inited_; };
    
private:
    GLuint id_;
    bool   inited_;
};

}