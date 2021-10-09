#include "GLGeoBatch.h"
#include <glm/vec4.hpp>

namespace DGL
{
GeoBatch::GeoBatch(std::initializer_list<Attribute> _attribs) {
	glCreateVertexArrays(1, &m_vao);

	int i = 0;
	int offset = 0;
	for (auto ite = _attribs.begin(); ite != _attribs.end(); ite++)
	{
		glVertexArrayAttribFormat(m_vao, i, ite->count, GL_FLOAT, false, offset);
		glVertexArrayAttribBinding(m_vao, i, 0);
		glEnableVertexArrayAttrib(m_vao, i);

		offset += sizeof(float) * ite->count;
		i++;
		m_attribs.push_back(*ite);
	}

	glVertexArrayVertexBuffer(m_vao, 0, m_vertex_buf.get_id(), 0, offset);
	glVertexArrayElementBuffer(m_vao, m_elem_buf.get_id());
	printf("geo batch init\n");
}

void GeoBatch::add_quad(float _width, float _height, std::string _name) {
	// m_geometries[_name] = { m_vertices.size(), 4 };

	int index_offset = get_vertices_count();
	
	auto add_point = [this](glm::vec4 _pos, glm::vec4 _uv, glm::vec4 _normal) {
		for (auto ite = m_attribs.begin(); ite != m_attribs.end(); ite++)
		{
			switch (ite->type)
			{
			case Attribute::POSITION:
				for (int i = 0; i < ite->count; i++)
					m_vertices.push_back((&_pos.x)[i]);
				break;
			case Attribute::UV:
				for (int i = 0; i < ite->count; i++)
					m_vertices.push_back((&_uv.x)[i]);
				break;
			case Attribute::NORMAL:
				for (int i = 0; i < ite->count; i++)
					m_vertices.push_back((&_normal.x)[i]);
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

	m_indices.emplace_back(0 + index_offset);
	m_indices.emplace_back(1 + index_offset);
	m_indices.emplace_back(2 + index_offset);
	m_indices.emplace_back(0 + index_offset);
	m_indices.emplace_back(2 + index_offset);
	m_indices.emplace_back(3 + index_offset);
}

void GeoBatch::upload() {
	int	vert_size	=	sizeof(float) * m_vertices.size();
	m_vertex_buf.allocate(vert_size, BufferFlag::DYNAMIC_STORAGE_BIT | BufferFlag::MAP_READ_BIT);
	m_vertex_buf.upload_data(vert_size, 0, m_vertices.data());

	int	elem_size	= sizeof(unsigned int) * m_indices.size();
	m_elem_buf.allocate(elem_size, BufferFlag::DYNAMIC_STORAGE_BIT | BufferFlag::MAP_READ_BIT);
	m_elem_buf.upload_data(elem_size, 0, m_indices.data());
}

void GeoBatch::draw_batch() {
	glBindVertexArray(m_vao);
	glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, (void*)0);
}

int GeoBatch::get_vertices_count() {
	int	vert_size = 0;	
	for (auto ite = m_attribs.begin(); ite != m_attribs.end(); ite++)
		vert_size += ite->count;

	return m_vertices.size() / vert_size;
}
}