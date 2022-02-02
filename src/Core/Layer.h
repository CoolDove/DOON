#pragma once
#include <string>
#include <Core/Image.h>
#include <DGLCore/GLTexture.h>
#include "Core/Color.h"
#include <Base/General.h>

enum class BlendMode : unsigned char {
    NORMAL,
    LIGHTEN
};

class Layer {
private:
    using ImagePtr = std::unique_ptr<Image>;
    using Tex2DPtr = std::unique_ptr<DGL::GLTexture2D>;
public:
    Layer(const std::string& path);
    Layer(unsigned int _width, unsigned int _height, std::string _name, Col_RGBA _col);
    ~Layer();

    // void mark_dirt(Dove::IRect2D _region);
    // void mark_dirt_whole();
    // void clear_dirt();


    uint32_t data_size() const { return sizeof(Col_RGBA) * info_.width * info_.height; }
    void update_tex();
    void mem_fetch();
    void mem_release();
public:
    struct {
        std::string name;
        BlendMode   blend_mode;
        uint32_t width;
        uint32_t height;
    } info_;

    // ImagePtr img_;
    Tex2DPtr tex_;
    Col_RGBA* pixels_;
};
