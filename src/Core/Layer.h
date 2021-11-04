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

class LayerImage {
private:
    using ImagePtr = std::unique_ptr<Image>;
    using Tex2DPtr = std::unique_ptr<DGL::GLTexture2D>;
public:
    LayerImage(int _width, int _height, Col_RGBA _col, bool _attach);
    ~LayerImage();

    ImagePtr img_;
    Tex2DPtr tex_;

    void mark_dirt(Dove::IRect2D _region);
    void clear_dirt();

    void update_tex(bool _whole);
    void attach_gltex();

    Dove::IRect2D get_dirt_region()   const { return dirt_region_; };
    bool          is_attached_gltex() const { return gl_attached_; };
private:
    Dove::IRect2D dirt_region_;
    bool          gl_attached_;
};

class Layer {
public:
    Layer(unsigned int _width, unsigned int _height, std::string _name, Col_RGBA _col);
    ~Layer();
public:
    struct {
        std::string name;
        BlendMode   blend_mode;
    } info_;

    // TODO: replace this with layer image
    Image img_;
    DGL::GLTexture2D tex_;
};

