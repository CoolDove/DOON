#pragma once
#include <unordered_map>
#include <DGLCore/GLProgram.h>

class Compositor {
private:
    using CompShaderMap = std::unordered_map<std::string, DGL::Program>;
public:
    Compositor();
    ~Compositor();

    void add_compshader(std::string _name, std::string _load_path);
    void compose();

    CompShaderMap comp_shaders_;
};
