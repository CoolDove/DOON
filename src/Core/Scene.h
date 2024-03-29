﻿#pragma once

#include "DGLCore/GLTexture.h"
#include <DGLCore/DGLCore.h>
#include <Core/Image.h>
#include <Core/Layer.h>
#include <Core/Color.h>
#include <Core/History.h>
#include <Base/General.h>

#include <list>
#include <memory>

using LayerList = std::list<std::unique_ptr<Layer>>;
using LayerIte = std::list<std::unique_ptr<Layer>>::iterator;
using LayerPtr = std::unique_ptr<Layer>;

using namespace DGL;

class Scene {
public:
    Scene(const char* _path);
    Scene(uint32_t _width, uint32_t _height, Col_RGBA _col);
    ~Scene();

    void on_render();
    void on_update();

    void add_layer(const std::string& _path);
    void add_layer(Col_RGBA _col, const std::string& _name);
    void add_layer(Col_RGBA _col);
    void change_layer(const std::string& _name);
    void change_layer(Layer* _layer);
    bool next_layer();
    bool previous_layer();

    void mark_region(Dove::IRect2D _region);
    void clear_region();

    bool composed_dirt_ = true;

    GLTexture2D*    get_composed_texture() {
        if (composed_dirt_)
            compose_texture();
        return &composed_texture_;
    }
    void compose_texture();
    
    Layer*        get_curr_layer() { return curr_layer_ite_->get(); }
    Dove::IRect2D get_region() const { return region_; }
    HistorySys*   get_history_sys() { return &history_sys_; }

    struct Fcol {
        float x;
        float y;
        float z;
        float w;
    } fbrush_color_ = {1};
    Col_RGBA brush_color_() {
        Col_RGBA color = {
            (unsigned char)(fbrush_color_.x * 0xff),
            (unsigned char)(fbrush_color_.y * 0xff),
            (unsigned char)(fbrush_color_.z * 0xff),
            (unsigned char)(fbrush_color_.w * 0xff)
        };
        return color;
    }

public:
    Camera      camera_;

    LayerList        layers_;
    DGL::GLTexture2D brush_layer_;
    DGL::GLTexture2D composed_texture_;

    struct {
        int width;
        int height;
    } info_;

    std::string save_path_ = "";
private:
    void create_scene(uint32_t _width, uint32_t _height, Col_RGBA _col);
    bool load_scene(const char* path);
    bool load_png(const char* path);

    bool load_doo(const char* path);
private:
    HistorySys    history_sys_;
    Dove::IRect2D region_; // updated region
    LayerIte      curr_layer_ite_;
};
