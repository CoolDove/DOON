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
	GLuint get_vao_id() { return m_vao; };
	std::vector<float>& get_vertices() { return m_vertices; };
	std::vector<unsigned int>& get_indices() { return m_indices; };
	Buffer* get_vertex_buffer() { return &m_vertex_buf; };
	Buffer* get_element_buffer() { return &m_elem_buf; };

private:
	int get_vertices_count();
private:
	std::vector<Attribute> m_attribs;
	std::vector<float> m_vertices;
	std::vector<GLuint> m_indices;

	// std::unordered_map<std::string, Geometry> m_geometries;
	
	Buffer m_vertex_buf;
	Buffer m_elem_buf;
	
	GLuint m_shader;
	GLuint m_vao;
};
}