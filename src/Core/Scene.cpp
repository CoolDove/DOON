#include "Scene.h"
#include "stb_image/stb_image.h"

Scene::Scene(const char* _image_path)
:   batch_({{DGL::Attribute::POSITION, 3}, { DGL::Attribute::UV, 2 }}),
    image_(_image_path, 0)
{

    camera_.position_.x = 0.0f;
    camera_.position_.y = 0.0f;
    camera_.size_       = 5.0f;

    batch_.add_quad(0.5f * image_.info_.width, 0.5f * image_.info_.height, "quad");
    batch_.upload();
}

Scene::~Scene() {
    // ...
}