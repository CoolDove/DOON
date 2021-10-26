#pragma once
#include <glm/vec2.hpp>
#include <glm/mat4x4.hpp>

namespace DGL
{
class Camera {
public:
	Camera();
public:
	glm::vec2 position_;
	float size_;
};
}