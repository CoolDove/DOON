#include "GLShader.h"
#include <fstream>
#include <stdlib.h>
#include <assert.h>
#include <DGLCore/DGLBase.h>

static char* read_file(const char* _path, int* _size = nullptr) {
    std::ifstream file(_path);

    if (!file.good())
        throw DGL::DGLERROR::FILE_NOT_EXIST;
    
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();

    char* buf = new char[size]();
    file.seekg(0, std::ios::beg);
    file.read(buf, size);
    file.close();

    if (_size != nullptr) *_size = size;
    
    return buf;
}

namespace DGL {
Shader::Shader() 
:   id_(0),
    inited_(false)
{}

Shader::Shader(const std::string& _path, ShaderType _type, std::string* _msg) 
:   id_(0),
    inited_(false)
{
    init(_type);
    load(_path, _msg);
}

Shader::~Shader() {
    if (!inited_) {
        glDeleteShader(id_);
    }
}
void Shader::init(ShaderType _type) {
    assert(!inited_&&"this shader has been initialized");
    id_ = glCreateShader(static_cast<GLenum>(_type));
    if (!id_) throw DGLERROR::CREATION_FAILED;

    type_   = _type;
    inited_ = true;
}

bool Shader::load(const std::string& _path, std::string* _compile_msg) {
    assert(inited_&&"shader has to be initialized before being loaded");

    char* src = read_file(_path.c_str());
    glShaderSource(id_, 1, &src, 0);

    bool result = compile(src, _compile_msg);

    delete [] src;
    return result;
}

bool Shader::compile(const std::string& _src, std::string* _compile_msg) {
    glCompileShader(id_);
    GLint compile_tag = 0;
    glGetShaderiv(id_, GL_COMPILE_STATUS, &compile_tag);
    if (compile_tag == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(id_, GL_INFO_LOG_LENGTH, &maxLength);

        char* infoLog = (char*)malloc(maxLength);
        glGetShaderInfoLog(id_, maxLength, &maxLength, &infoLog[0]);
        if (_compile_msg != nullptr) {
            _compile_msg->clear();
            *_compile_msg = infoLog;
        }
        glDeleteShader(id_);
        free(infoLog);

        throw DGLERROR::SHADER_COMPILING_FAILED;
    }

    return compile_tag;
}

}