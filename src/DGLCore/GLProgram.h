#pragma once

#include "GLShader.h"

#include <glad/glad.h>

#include <string>
#include <vector>
#include <unordered_map>
#include <stdint.h>
#include <stdarg.h>

namespace DGL
{
class Program {
public:
    Program();
    ~Program();

    void init();
    // void link(std::vector<const Shader*> _shaders);

    /// fill in shader pointers
    void link(int _count, ... );
    void bind();

public:
    // you have to manually bind this program before setting uniforms
    void uniform_i(const std::string& _name, int32_t _v0);
    void uniform_i(const std::string& _name, int32_t _v0, int32_t _v1);
    void uniform_i(const std::string& _name, int32_t _v0, int32_t _v1, int32_t _v2);
    void uniform_i(const std::string& _name, int32_t _v0, int32_t _v1, int32_t _v2, int32_t _v3);

    void uniform_f(const std::string& _name, float _v0);
    void uniform_f(const std::string& _name, float _v0, float _v1);
    void uniform_f(const std::string& _name, float _v0, float _v1, float _v2);
    void uniform_f(const std::string& _name, float _v0, float _v1, float _v2, float _v3);

    void uniform_ui(const std::string& _name, uint32_t _v0);
    void uniform_ui(const std::string& _name, uint32_t _v0, uint32_t _v1);
    void uniform_ui(const std::string& _name, uint32_t _v0, uint32_t _v1, uint32_t _v2);
    void uniform_ui(const std::string& _name, uint32_t _v0, uint32_t _v1, uint32_t _v2, uint32_t _v3);

    void uniform_mat(const std::string& _name, uint32_t _dimensions, const float* _data, bool _transpose = false, int _count = 1);

private:
    GLint locate_uniform(const std::string& _name);
public:
    uint32_t get_glid()   const { return id_; };
    bool     get_inited() const { return inited_; };
    bool     good() const { return good_; };
    
private:
    GLuint id_;
    bool   inited_;
    bool   good_;

    std::unordered_map<std::string, GLuint> uniform_location_cache_;
};

}
