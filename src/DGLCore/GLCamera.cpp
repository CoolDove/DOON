#include "GLCamera.h"
// #include <glm/ext/matrix_transform.hpp>

namespace DGL
{
Camera::Camera() : m_position(0, 0) {
}

glm::mat4 Camera::calc_view() {
	glm::mat4 view(0);
	view[0][0] = 1.0f;
	view[1][1] = 1.0f;
	view[2][2] = 1.0f;
	view[3][3] = 1.0f;

	view[3][0] = -m_position.x;
	view[3][1] = -m_position.y;
	return view;
}

glm::mat4 Camera::calc_proj(int _width, int _height) {
	glm::mat4 proj(0);
	proj[0][0] = 2.0f/(_width * m_size);
	proj[1][1] = 2.0f/(_height * m_size);
	proj[2][2] = 1;
	proj[3][3] = 1;
	return proj;
}
}