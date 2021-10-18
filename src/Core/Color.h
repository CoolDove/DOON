#pragma once

struct Col_RGB {
    union {
        struct {
            unsigned char r;
            unsigned char g;
            unsigned char b;
        };
        unsigned int cluster;
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
        unsigned int cluster;
    };
};

// struct Col_HSV {
//     union {
//         struct {
//             unsigned char h;
//             unsigned char s;
//             unsigned char v;
//         };
//         unsigned int cluster;
//     };
// };
