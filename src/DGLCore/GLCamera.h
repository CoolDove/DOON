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
	glm::vec2 get_pos() { return position_; }
	void set_pos(glm::vec2 _pos) { position_ = _pos; }
	float get_size() { return size_; }
	void set_size(float _size) { size_ = _size; }
private:
	glm::vec2 position_;
	float size_;
};
}