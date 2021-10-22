#include "GLProgram.h"
#include "DGLBase.h"

namespace DGL
{
Program::Program()
:   id_(0),
    inited_(false)
{
}

Program::~Program() {
    glDeleteProgram(id_);
}

void Program::init() {
    assert(!inited_);
    id_ = glCreateProgram();
    if (!id_) throw DGLERROR::CREATION_FAILED;
    inited_ = true;
}

void Program::link(std::vector<Shader*> _shaders) {
    if (!inited_) init();

    uint16_t mask = 0u;
    auto check_mask = [&mask](uint16_t _bit_pos){
        if (mask & 1u<<_bit_pos) 
            throw DGLERROR::LINK_UNINITED_SHADER;
        else 
            mask = mask | 1u<<_bit_pos;
    };

    for (uint16_t i = 0; i < _shaders.size(); i++)
    {
        switch (_shaders[i]->get_type())
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

        glAttachShader(id_, _shaders[i]->get_glid());
    }

    glLinkProgram(id_);

    GLint status;
    glGetProgramiv(id_, GL_LINK_STATUS, &status);
    assert(status);
}
void Program::bind() {
    glUseProgram(id_);
}


}