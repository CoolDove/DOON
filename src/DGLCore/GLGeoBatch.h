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
	struct Geometry {
		size_t offset;
		size_t count;
	};
	GeoBatch(std::initializer_list<Attribute> _attribs);

	void draw_batch();
	void add_quad(float _width, float _height, std::string _name);
	int get_geometry(const std::string& _name);

	void upload();

public:
	GLuint get_vao_id() { return vao_; };
	std::vector<float>& get_vertices() { return vertices_; };
	std::vector<unsigned int>& get_indices() { return indices_; };
	Buffer* get_vertex_buffer() { return &vertex_buf_; };
	Buffer* get_element_buffer() { return &elem_buf_; };

private:
	int get_vertices_count();
private:
	std::vector<Attribute> attribs_;
	std::vector<float> vertices_;
	std::vector<GLuint> indices_;

	// std::unordered_map<std::string, Geometry> m_geometries;
	
	Buffer vertex_buf_;
	Buffer elem_buf_;
	
	GLuint shader_;
	GLuint vao_;
};
}