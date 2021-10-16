#include "Scene.h"
#include "stb_image/stb_image.h"

Scene::Scene(const char* _image_path)
:   image_(_image_path, 0)
{
    camera_.position_.x = 0.0f;
    camera_.position_.y = 0.0f;
    camera_.size_       = 5.0f;
}

Scene::Scene(unsigned int _base_color)
:   image_(512, 512, _base_color)
{
    camera_.position_.x = 0.0f;
    camera_.position_.y = 0.0f;
    camera_.size_       = 5.0f;
}
Scene::~Scene() {
    // ...
}