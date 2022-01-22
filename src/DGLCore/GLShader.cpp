#include "GLShader.h"
#include <fstream>
#include <ios>
#include <stdlib.h>
#include <assert.h>
#include "GLDebugger.h"

static char* read_file(const char* _path, int* _size = nullptr) {
    std::ifstream file(_path, std::ios_base::in);

    bool good = file.good();

    if (!file.good()) {
        // throw DGL::EXCEPTION::FILE_NOT_EXIST(_path);
        return nullptr;
    }
    
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
    inited_(false),
    good_(false)
{}

Shader::Shader(const std::string& _path, ShaderType _type, std::string* _msg) 
:   id_(0),
    inited_(false),
    good_(false)
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
    if (!id_) throw DGL::EXCEPTION::CREATION_FAILED();

    type_   = _type;
    inited_ = true;
}

bool Shader::load(const std::string& _path, std::string* _compile_msg) {
    assert(inited_&&"shader has to be initialized before being loaded");

    char* src = read_file(_path.c_str());
    if (src == nullptr) return false;

    glShaderSource(id_, 1, &src, 0);

    bool result = compile(src, _compile_msg);// true means good

    if (result) good_ = true;

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
        std::string log = infoLog;
        glDeleteShader(id_);
        DLOG_ERROR("%s", infoLog);

        free(infoLog);
        // throw DGL::EXCEPTION::SHADER_COMPILING_FAILED(log);
    }

    return compile_tag;
}

// void Shader::dispatch(uint32_t _x, uint32_t _y, uint32_t _z) {
    // assert(type_ == ShaderType::COMPUTE_SHADER && "only compute shader can be dispatched!!!");
    // bind();
    // glDispatchCompute(_x, _y, _z);
// }
}
