#include "Scene.h"
#include "stb_image/stb_image.h"
#include <Core/Sampler.h>

// Scene::Scene(const char* _image_path)
// :   image_(_image_path, 0)
// {
//     camera_.position_.x = 0.0f;
//     camera_.position_.y = 0.0f;
//     camera_.size_       = 5.0f;

//     info_.width  = image_.info_.width;
//     info_.height = image_.info_.height;
// }

Scene::Scene(unsigned int _width, unsigned int _height, Col_RGBA _col)
:   image_(_width, _height, _col)
{
    camera_.position_.x = 0.0f;
    camera_.position_.y = 0.0f;
    camera_.size_       = 5.0f;

    info_.width  = image_.info_.width;
    info_.height = image_.info_.height;

    add_layer(_col);
    update({0, 0, info_.width, info_.height});
}

void Scene::update(RectInt _region) {
    // update the datas
    for (uint32_t iy = _region.posy; iy < _region.posy + _region.height; iy++)
    {
        for (uint32_t ix = _region.posx; ix < _region.posx + _region.width; ix++)
        {
            uint32_t byte_index = (iy * info_.width + ix) * 4;
            Col_RGBA dcol = {0};

            auto byte2float = [](uint32_t _in)->float{return (float)_in / 255;};
            auto float2byte = [](float _in)->char{return (char)_in * 255;};

            for (auto ite = layers_.cbegin(); ite != layers_.cend(); ite++)
            {
                Col_RGBA* layer_pix = (Col_RGBA*)ite->get()->img_.pixels_;
                layer_pix += info_.width * iy + ix;

                switch (ite->get()->info_.blend_mode)
                {
                case BlendMode::NORMAL:
                {
                    float srgb = byte2float(layer_pix->a);
                    float sa   = byte2float(layer_pix->a);
                    float drgb = 1 - byte2float(layer_pix->a);
                    float da   = 1 - byte2float(layer_pix->a);

                    dcol.r = float2byte(byte2float(layer_pix->r) * srgb + byte2float(dcol.r) * drgb);
                    dcol.g = float2byte(byte2float(layer_pix->g) * srgb + byte2float(dcol.g) * drgb);
                    dcol.b = float2byte(byte2float(layer_pix->b) * srgb + byte2float(dcol.b) * drgb);
                    dcol.a = float2byte(byte2float(layer_pix->a) * sa + byte2float(dcol.a) * da);
                    
                } break;
                case BlendMode::LIGHTEN:
                {
                    dcol = {0xff, 0x00, 0xff, 0xff};
                } break;
                default:
                    break;
                }
            }

            Col_RGBA* target_pix = (Col_RGBA*)&image_.pixels_[byte_index];
            *target_pix = dcol;
        }
    }

    // assign a updated region
    region_ = _region;
}

void Scene::comfirm_update() {
    memset(&region_, 0, sizeof(RectInt));
}

void Scene::add_layer(Col_RGBA _col) {
    layers_.emplace_back(std::make_unique<Layer>(
        info_.width,
        info_.height,
        "base",
        _col
    ));

    
    curr_layer_ = (--layers_.end())->get();
}

Scene::~Scene() {
    // ...
}