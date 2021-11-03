#pragma once
#include <stdint.h>

#define out__
#define in__

// TODO: finish this file, uvector things...

namespace Dove {
struct UVector2D {
    union {
        uint32_t r;
        uint32_t x;
        uint32_t width;
    };
    union {
        uint32_t g;
        uint32_t y;
        uint32_t height;
    };
};

struct IVector2D {
    union {
        int r;
        int x;
    };
    union {
        int g;
        int y;
    };
};

struct IVector3D {
    union {
        int r;
        int x;
    };
    union {
        int g;
        int y;
    };
    union {
        int b;
        int z;
    };
};

struct IVector4D {
    union {
        int r;
        int x;
    };
    union {
        int g;
        int y;
    };
    union {
        int b;
        int z;
    };
    union {
        int a;
        int w;
    };
};

struct IRect2D {
// NOTE: pos is left-up corner position
    union {
        IVector2D position;
        struct {
            int posx;
            int posy;
        };
    };
    union {
        UVector2D size;
        struct {
            uint32_t width;
            uint32_t height;
        };
    };
};

struct IAABBox2D {
    union {
        IVector2D min;
        struct {
            int minx;
            int miny;
        };
    };
    union {
        IVector2D max;
        struct {
            int maxx;
            int maxy;
        };
    };
};

inline IAABBox2D rect_to_aabb(IRect2D _rect) {
    IAABBox2D box = {0};
    box.minx = _rect.posx;
    box.miny = _rect.posy;

    box.maxx = _rect.posx + _rect.width;
    box.maxy = _rect.posy + _rect.height;
    return box;
}

inline IRect2D aabb_to_rect(IAABBox2D _aabb) {
    IRect2D rect = {0};
    rect.position = {_aabb.minx, _aabb.miny};
    rect.width  = _aabb.maxx - _aabb.minx;
    rect.height = _aabb.maxy - _aabb.miny;
    return rect;
}

inline IAABBox2D merge_aabb(IAABBox2D _a, IAABBox2D _b) {
    int minx = _a.minx < _b.minx ? _a.minx : _b.minx;
    int miny = _a.miny < _b.miny ? _a.miny : _b.miny;

    int maxx = _a.maxx > _b.maxx ? _a.maxx : _b.maxx;
    int maxy = _a.maxy > _b.maxy ? _a.maxy : _b.maxy;

    IAABBox2D aabb = {0};
    aabb.min = {minx, miny};
    aabb.max = {maxx, maxy};
    return aabb;
}

inline IRect2D merge_rect(IRect2D _a, IRect2D _b) {
// NOTE: may be some problems here
    IAABBox2D boxa = rect_to_aabb(_a);
    IAABBox2D boxb = rect_to_aabb(_b);
    return aabb_to_rect(merge_aabb(boxa, boxb));
}
}
