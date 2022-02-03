#include "GLProgram.h"
#include "GLDebugger.h"

namespace DGL
{
Program::Program()
:   id_(0),
    inited_(false),
    good_(false)
{
}

Program::~Program() {
    glDeleteProgram(id_);
}

void Program::init() {
    assert(!inited_);
    id_ = glCreateProgram();
    if (!id_) throw DGL::EXCEPTION::CREATION_FAILED();
    inited_ = true;
}

void Program::link(int _count, ... ) {
    if (!inited_) init();

    uint16_t mask = 0u;
    auto check_mask = [&mask](uint16_t _bit_pos){
        if (mask & 1u<<_bit_pos) 
            throw DGL::EXCEPTION::LINK_UNINITED_SHADER();
        else 
            mask = mask | 1u<<_bit_pos;
    };

    va_list args;
    va_start(args, _count); 

    const Shader* shader = nullptr;

    for (int i = 0; i < _count; i++) {
        shader = va_arg(args, const Shader*);

        if (!shader->good()) {
            DLOG_ERROR("shader is not good, failed to link program");
            return;
        }
        
        switch (shader->get_type())
        {
        case ShaderType::FRAGMENT_SHADER:
            check_mask(1);
            break;
        case ShaderType::VERTEX_SHADER:
            check_mask(2);
            break;
        case ShaderType::COMPUTE_SHADER:
            check_mask(3);
            break;
        case ShaderType::TESS_CONTROL_SHADER:
            check_mask(4);
            break;
        case ShaderType::TESS_EVALUATION_SHADER:
            check_mask(5);
            break;
        case ShaderType::GEOMETRY_SHADER:
            check_mask(6);
            break;
        default:
            break;
        }

        glAttachShader(id_, shader->get_glid());
    }
    va_end(args);

    glLinkProgram(id_);

    GLint status;
    glGetProgramiv(id_, GL_LINK_STATUS, &status);
    assert(status && "failed to link program");
    good_ = true;
}
void Program::bind() {
    if (!good_) {
        DLOG_ERROR("program is not good, cannot be bind");
        return;
    }
    glUseProgram(id_);
}

GLint Program::locate_uniform(const std::string& _name) {
    if (uniform_location_cache_.find(_name) == uniform_location_cache_.end()) {
        // we didnt find the location in the cache
        GLuint loc = glGetUniformLocation(id_, _name.c_str());

        // if (loc == -1) throw DGL::EXCEPTION::NO_UNIFORM_LOCATION_FOUND(_name);
        if (loc == -1) {
            DLOG_ERROR("uniform name %s not found", _name.c_str());
            return -1;
        }

        uniform_location_cache_[_name] = loc;
        return loc;
    } else {
        return uniform_location_cache_[_name];
    }
}

void Program::uniform_i(const std::string& _name, int32_t _v0){
    glUniform1i(locate_uniform(_name), _v0);
}

void Program::uniform_i(const std::string& _name, int32_t _v0, int32_t _v1){
    glUniform2i(locate_uniform(_name), _v0, _v1);
}
void Program::uniform_i(const std::string& _name, int32_t _v0, int32_t _v1, int32_t _v2){
    glUniform3i(locate_uniform(_name), _v0, _v1, _v2);
}
void Program::uniform_i(const std::string& _name, int32_t _v0, int32_t _v1, int32_t _v2, int32_t _v3){
    glUniform4i(locate_uniform(_name), _v0, _v1, _v2, _v3);
}

void Program::uniform_f(const std::string& _name, float _v0){
    glUniform1f(locate_uniform(_name), _v0);
}
void Program::uniform_f(const std::string& _name, float _v0, float _v1){
    glUniform2f(locate_uniform(_name), _v0, _v1);
}
void Program::uniform_f(const std::string& _name, float _v0, float _v1, float _v2){
    glUniform3f(locate_uniform(_name), _v0, _v1, _v2);
}
void Program::uniform_f(const std::string& _name, float _v0, float _v1, float _v2, float _v3){
    glUniform4f(locate_uniform(_name), _v0, _v1, _v2, _v3);
}

void Program::uniform_ui(const std::string& _name, uint32_t _v0){
    glUniform1ui(locate_uniform(_name), _v0);
}
void Program::uniform_ui(const std::string& _name, uint32_t _v0, uint32_t _v1){
    glUniform2ui(locate_uniform(_name), _v0, _v1);
}
void Program::uniform_ui(const std::string& _name, uint32_t _v0, uint32_t _v1, uint32_t _v2){
    glUniform3ui(locate_uniform(_name), _v0, _v1, _v2);
}
void Program::uniform_ui(const std::string& _name, uint32_t _v0, uint32_t _v1, uint32_t _v2, uint32_t _v3){
    glUniform4ui(locate_uniform(_name), _v0, _v1, _v2, _v3);
}

void Program::uniform_mat(const std::string& _name, uint32_t _dimensions, const float* _data, bool _transpose, int _count){
    if (_dimensions > 4) _dimensions = 4;
    if (_dimensions < 2) _dimensions = 2;

    switch (_dimensions)
    {
    case 2:
        glUniformMatrix2fv(locate_uniform(_name), _count, _transpose, _data);
        break;
    case 3:
        glUniformMatrix3fv(locate_uniform(_name), _count, _transpose, _data);
        break;
    case 4:
        glUniformMatrix4fv(locate_uniform(_name), _count, _transpose, _data);
        break;
    default:
        break;
    }
}

}
