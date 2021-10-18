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
