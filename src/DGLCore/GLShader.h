#pragma once
#include <glad/glad.h>
#include <string>
#include <stdint.h>

namespace DGL
{
enum class ShaderType : uint32_t {
    FRAGMENT_SHADER        = 0x8B30,
    VERTEX_SHADER          = 0x8B31,
    COMPUTE_SHADER         = 0x91B9,
    TESS_CONTROL_SHADER    = 0x8E88,
    TESS_EVALUATION_SHADER = 0x8E87,
    GEOMETRY_SHADER        = 0x8DD9
};

class Shader {
public:
    Shader();
    Shader(const std::string& _path, ShaderType _type, std::string* _msg = nullptr);
    ~Shader();

    // true means successfully compiled, false means not
    void init(ShaderType _type);
    bool load(const std::string& _path, std::string* _compile_msg = nullptr);
    bool compile(const std::string& _src, std::string* _compile_msg = nullptr);

    void bind() { glUseProgram(id_); }
public:
    uint32_t   get_glid() const { return id_; }
    ShaderType get_type() const { return type_; }
    bool       get_inited() const { return inited_; }

private:
    GLuint      id_;
    bool        inited_;
    ShaderType  type_;
};
}