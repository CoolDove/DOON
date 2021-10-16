#include "GLGeoBatch.h"
#include <glm/vec4.hpp>

namespace DGL
{
GeoBatch::GeoBatch()
:   buffer_inited_(false)
{
}

// GeoBatch::GeoBatch(std::initializer_list<Attribute> _attribs)
// :   buffer_inited_(false)
// {
//     glCreateVertexArrays(1, &vao_);

//     int i = 0;
//     int offset = 0;
//     for (auto ite = _attribs.begin(); ite != _attribs.end(); ite++)
//     {
//         glVertexArrayAttribFormat(vao_, i, ite->count, GL_FLOAT, false, offset);
//         glVertexArrayAttribBinding(vao_, i, 0);
//         glEnableVertexArrayAttrib(vao_, i);

//         offset += sizeof(float) * ite->count;
//         i++;
//         attribs_.push_back(*ite);
//     }
//     glVertexArrayVertexBuffer(vao_, 0, vertex_buf_.get_id(), 0, offset);
//     glVertexArrayElementBuffer(vao_, index_buf_.get_id());

//     printf("geo batch init\n");
// }

void GeoBatch::init(std::initializer_list<Attribute> _attribs) {
    vertex_buf_.init();
    index_buf_.init();
    
    glCreateVertexArrays(1, &vao_);

    int i = 0;
    int offset = 0;
    for (auto ite = _attribs.begin(); ite != _attribs.end(); ite++)
    {
        glVertexArrayAttribFormat(vao_, i, ite->count, GL_FLOAT, false, offset);
        glVertexArrayAttribBinding(vao_, i, 0);
        glEnableVertexArrayAttrib(vao_, i);

        offset += sizeof(float) * ite->count;
        i++;
        attribs_.push_back(*ite);
    }
    glVertexArrayVertexBuffer(vao_, 0, vertex_buf_.get_id(), 0, offset);
    glVertexArrayElementBuffer(vao_, index_buf_.get_id());

    printf("geo batch init\n");
}

void GeoBatch::add_quad(float _width, float _height, std::string _name) {
    int index_offset = get_vertices_count();
    
    auto add_point = [this](glm::vec4 _pos, glm::vec4 _uv, glm::vec4 _normal) {
        for (auto ite = attribs_.begin(); ite != attribs_.end(); ite++)
        {
            switch (ite->type)
            {
            case Attribute::POSITION:
                for (int i = 0; i < ite->count; i++)
                    vertices_.push_back((&_pos.x)[i]);
                break;
            case Attribute::UV:
                for (int i = 0; i < ite->count; i++)
                    vertices_.push_back((&_uv.x)[i]);
                break;
            case Attribute::NORMAL:
                for (int i = 0; i < ite->count; i++)
                    vertices_.push_back((&_normal.x)[i]);
                break;
            default:
                break;
            }
        }
    };

    add_point(glm::vec4(-_width, -_height, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
    add_point(glm::vec4(-_width,  _height, 0.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
    add_point(glm::vec4( _width,  _height, 0.0f, 0.0f), glm::vec4(1.0f, 1.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));
    add_point(glm::vec4( _width, -_height, 0.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 0.0f));

    indices_.emplace_back(0 + index_offset);
    indices_.emplace_back(1 + index_offset);
    indices_.emplace_back(2 + index_offset);
    indices_.emplace_back(0 + index_offset);
    indices_.emplace_back(2 + index_offset);
    indices_.emplace_back(3 + index_offset);
}

void GeoBatch::clear() {
    vertices_.clear();
    indices_.clear();
}

void GeoBatch::upload() {
    int	vert_size = sizeof(float) * vertices_.size();
    vertex_buf_.allocate(vert_size, BufferFlag::DYNAMIC_STORAGE_BIT | BufferFlag::MAP_READ_BIT);
    vertex_buf_.upload_data(vert_size, 0, vertices_.data());

    int	elem_size = sizeof(unsigned int) * indices_.size();
    index_buf_.allocate(elem_size, BufferFlag::DYNAMIC_STORAGE_BIT | BufferFlag::MAP_READ_BIT);
    index_buf_.upload_data(elem_size, 0, indices_.data());
}

void GeoBatch::draw_batch() {
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, (void*)0);
}

int GeoBatch::get_vertices_count() {
    int	vert_size = 0;	
    for (auto ite = attribs_.begin(); ite != attribs_.end(); ite++)
        vert_size += ite->count;

    return vertices_.size() / vert_size;
}
}