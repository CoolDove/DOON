#pragma once
#include <glad/glad.h>
#include <string>

namespace DGL
{
class Shader {
public:
    // true means successfully compiled, false means not
    bool load(const std::string _vert_path, const std::string _frag_path);
    void bind() { glUseProgram(id_); }
public:
    GLuint get_id() { return id_; }
private:
    GLuint id_;
};
}