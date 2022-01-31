#pragma once
#include <stdint.h>

struct Col_RGB {
    union {
        struct {
            unsigned char r;
            unsigned char g;
            unsigned char b;
        };
        uint32_t cluster;
    };
};

struct Col_RGBA {
    union {
        struct {
            unsigned char r;
            unsigned char g;
            unsigned char b;
            unsigned char a;
        };
        uint32_t cluster;
    };
};

struct FCol_RGB {
    float r;
    float g;
    float b;
};

struct FCol_RGBA {
    float r;
    float g;
    float b;
    float a;
};

inline FCol_RGBA get_float_col(Col_RGBA _col) {
    FCol_RGBA col = {0};
    col.r = (float)_col.r / 255.0f;
    col.g = (float)_col.g / 255.0f;
    col.b = (float)_col.b / 255.0f;
    col.a = (float)_col.a / 255.0f;
    return col;
}

inline FCol_RGB get_float_col(Col_RGB _col) {
    FCol_RGB col = {0};
    col.r = (float)_col.r / 255.0f;
    col.g = (float)_col.g / 255.0f;
    col.b = (float)_col.b / 255.0f;
    return col;
}


inline void get_float_col(Col_RGB _col, float* r, float* g, float* b) {
}

inline void get_float_col(Col_RGB _col, float* fcol, uint8_t channels) {
    for (int i = 0; i < channels; i++) {
        fcol[i] = (float)*((unsigned char*)&_col + i) / 255.0f;
    }
}

inline void get_float_col(Col_RGBA _col, float* r, float* g, float* b, float* a) {
}
inline void get_float_col(Col_RGBA _col, float* col) {
}
// struct Col_HSV {
//     union {
//         struct {
//             unsigned char h;
//             unsigned char s;
//             unsigned char v;
//         };
//         uint32_t cluster;
//     };
// };
