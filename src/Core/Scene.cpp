#include "Scene.h"
#include "stb_image/stb_image.h"
#include <Core/Sampler.h>
#include <DoveLog.hpp>

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
    for (uint32_t iy = _region.posy; iy < _region.posy + _region.height; iy++) {
        uint32_t byte_index = (iy * info_.width + _region.posx) * 4;

        Col_RGBA* dst = (Col_RGBA*)&image_.pixels_[byte_index];
        Col_RGBA* src = (Col_RGBA*)&layers_.front().get()->img_.pixels_[byte_index];
        memcpy(dst, src, _region.width * 4);

        DLOG_TRACE("update %d %d %d %d", _region.posx, _region.posy, _region.width, _region.height);
    }

    if (layers_.size() == 1) {
        region_ = _region;
        return;
    }

    for (uint32_t iy = _region.posy; iy < _region.posy + _region.height; iy++) {
        for (uint32_t ix = _region.posx; ix < _region.posx + _region.width; ix++) {
            uint32_t byte_index = (iy * info_.width + ix) * 4;
            // Col_RGBA dcol = {1};
            Col_RGBA dcol = *(Col_RGBA*)&layers_.front().get()->img_.pixels_[byte_index];

            auto byte2float = [](uint32_t _in)->float{return (float)_in / 255;};
            auto float2byte = [](float _in)->char{return _in * 255;};

            for (auto ite = (++layers_.cbegin()); ite != layers_.cend(); ite++)
            {
                if (ite == layers_.cbegin()) {
                    continue;
                }

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

                    float src[4] = {
                        byte2float(layer_pix->r),
                        byte2float(layer_pix->g),
                        byte2float(layer_pix->b),
                        byte2float(layer_pix->a)
                    };

                    float dst[4] = {
                        byte2float(dcol.r),
                        byte2float(dcol.g),
                        byte2float(dcol.b),
                        byte2float(dcol.a)
                    };

                    dcol.r = float2byte(src[0] * srgb + dst[0] * drgb);
                    dcol.g = float2byte(src[1] * srgb + dst[1] * drgb);
                    dcol.b = float2byte(src[2] * srgb + dst[2] * drgb);
                    dcol.a = float2byte(src[3] * sa   + dst[3] * da  );
                    
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
    if (layers_.size() == 0) {
        add_layer(_col, "base");
    } else {
        add_layer(_col, "layer" + std::to_string(layers_.size() - 1));
    }
}
void Scene::add_layer(Col_RGBA _col, const std::string& _name) {
    layers_.emplace_back(std::make_unique<Layer>(
        info_.width,
        info_.height,
        _name,
        _col
    ));
    
    curr_layer_ = (--layers_.end())->get();
}

Scene::~Scene() {
    // ...
}