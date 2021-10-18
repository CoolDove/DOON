#include "Scene.h"
#include "stb_image/stb_image.h"

Scene::Scene(const char* _image_path)
:   image_(_image_path, 0)
{
    camera_.position_.x = 0.0f;
    camera_.position_.y = 0.0f;
    camera_.size_       = 5.0f;

    info_.width  = image_.info_.width;
    info_.height = image_.info_.height;

}

Scene::Scene(unsigned int _width, unsigned int _height, unsigned int _base_color)
:   image_(_width, _height, _base_color)
{
    camera_.position_.x = 0.0f;
    camera_.position_.y = 0.0f;
    camera_.size_       = 5.0f;

    info_.width  = image_.info_.width;
    info_.height = image_.info_.height;
}

void Scene::update(RectInt _region) {
    // update the datas

    // assign a updated region
    region_ = _region;
}
void Scene::comfirm_update() {
    memset(&region_, 0, sizeof(RectInt));
}

void Scene::add_layer() {
    layers_.emplace_back();
}

Scene::~Scene() {
    // ...
}