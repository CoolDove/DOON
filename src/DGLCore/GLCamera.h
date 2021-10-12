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
	glm::vec2 position_;
	float size_;
};
}