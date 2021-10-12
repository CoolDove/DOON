#include "GLCamera.h"
// #include <glm/ext/matrix_transform.hpp>

namespace DGL
{
Camera::Camera() : position_(0, 0) {
}

glm::mat4 Camera::calc_view() {
	glm::mat4 view(0);
	view[0][0] = 1.0f;
	view[1][1] = 1.0f;
	view[2][2] = 1.0f;
	view[3][3] = 1.0f;

	view[3][0] = -position_.x;
	view[3][1] = -position_.y;
	return view;
}

glm::mat4 Camera::calc_proj(int _width, int _height) {
	glm::mat4 proj(0);
	proj[0][0] = 2.0f * size_ /(_width);
	proj[1][1] = 2.0f * size_ /(_height);
	proj[2][2] = 1;
	proj[3][3] = 1;
	return proj;
}
}