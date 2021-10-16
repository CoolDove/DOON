#pragma once
#include <glad/glad.h>
#include <vector>
#include <unordered_map>
#include <initializer_list>
#include <string>
#include "GLShader.h"
#include "GLBuffer.h"

namespace DGL
{
struct Attribute {
    enum Type {
        POSITION,
        UV,
        NORMAL
    } type;
    int count;
};

class GeoBatch {
public:
    GeoBatch();
    // GeoBatch(std::initializer_list<Attribute> _attribs);

    void init(std::initializer_list<Attribute> _attribs);
    void draw_batch();
    void add_quad(float _width, float _height, std::string _name);
    int  get_geometry(const std::string& _name);

    void clear();
    void upload();

public:
    GLuint get_vao_id() { return vao_; };

public:
    std::vector<Attribute> 	attribs_;
    std::vector<float> 	    vertices_;
    std::vector<GLuint>     indices_;
    
    Buffer vertex_buf_;
    Buffer index_buf_;
    
    GLuint shader_;

private:
    int get_vertices_count();
private:
    GLuint vao_;
    bool   buffer_inited_;
};
}