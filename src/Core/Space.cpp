#include "Space.h"

glm::mat4 Space::mat_world_camera(const DGL::Camera* _cam) {
	glm::mat4 view(0);
	view[0][0] = 1.0f;
	view[1][1] = 1.0f;
	view[2][2] = 1.0f;
	view[3][3] = 1.0f;

	view[3][0] = -_cam->position_.x;
	view[3][1] = -_cam->position_.y;
	return view;
}

glm::mat4 Space::mat_camera_world(const DGL::Camera* _cam) {
	glm::mat4 view(0);
	view[0][0] = 1.0f;
	view[1][1] = 1.0f;
	view[2][2] = 1.0f;
	view[3][3] = 1.0f;

	view[3][0] = -_cam->position_.x;
	view[3][1] = -_cam->position_.y;
	return glm::inverse(view);
}

glm::mat4 Space::mat_camproj(const DGL::Camera* _cam, int _width, int _height) {
	glm::mat4 proj(0);
	proj[0][0] = 2.0f * _cam->size_ /(_width);
	proj[1][1] = 2.0f * _cam->size_ /(_height);
	proj[2][2] = 1;
	proj[3][3] = 1;
	return proj;
}


glm::mat4 Space::mat_world_ndc(const DGL::Camera* _cam, int _width, int _height) {
    return mat_world_camera(_cam) * mat_camproj(_cam, _width, _height);
}

glm::mat4 Space::mat_ndc_world(const DGL::Camera* _cam, int _width, int _height) {
    return glm::inverse(mat_world_camera(_cam) * mat_camproj(_cam, _width, _height));
}

// glm::mat4 Space::mat_texture_uv(int _width, int _height) {

// }

// glm::mat4 Space::mat_uv_texture(int _width, int _height) {

// }

// glm::mat4 Space::mat_brush_world(int _width, int _height) {

// }

// glm::mat4 Space::mat_world_brush(int _width, int _height) {

// }