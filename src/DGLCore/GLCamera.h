#pragma once
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

namespace DGL
{
class Camera {
public:
	Camera();

	glm::mat4 calc_view();
	glm::mat4 calc_proj(int _width, int _height);
public:
	glm::vec2 get_pos() { return m_position; }
	void set_pos(glm::vec2 _pos) { m_position = _pos; }
	float get_size() { return m_size; }
	void set_size(float _size) { m_size = _size; }
private:
	glm::vec2 m_position;
	float m_size;
};
}