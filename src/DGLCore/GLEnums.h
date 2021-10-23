#pragma once
#include <stdint.h>

namespace DGL
{
enum class Access : uint32_t {
    READ_ONLY  = 0x88B8,
    WRITE_ONLY = 0x88B9,
    READ_WRITE = 0x88BA
};
    
enum class SizedInternalFormat : uint32_t {
    R8             = 0x8229,
    R8_SNORM       = 0x8F94,
    R16            = 0x822A,
    R16_SNORM      = 0x8F98,
    RG8            = 0x822B,
    RG8_SNORM      = 0x8F95,
    RG16           = 0x822C,
    RG16_SNORM     = 0x8F99,
    R3_G3_B2       = 0x2A10,

    RGB4           = 0x804F,
    RGB5           = 0x8050,
    RGB8           = 0x8051,

    RGB8_SNORM     = 0x8F96,
    RGB10          = 0x8052,
    RGB12          = 0x8053,
    RGB16_SNORM    = 0x8F9A,

    RGBA2          = 0x8055,
    RGBA4          = 0x8056,
    RGB5_A1        = 0x8057,


    RGBA8          = 0x8058,
    RGBA8_SNORM    = 0x8F97,

    RGB10_A2       = 0x8059,
    RGB10_A2UI     = 0x906F,

    RGBA12         = 0x805A,
    RGBA16         = 0x805B,

    SRGB8          = 0x8C41,
    SRGB8_ALPHA8   = 0x8C43,

    R16F           = 0x822D,
    RG16F          = 0x822F,
    RGB16F         = 0x881B,
    RGBA16F        = 0x881A,

    R32F           = 0x822E,
    RG32F          = 0x8230,
    RGB32F         = 0x8815,
    RGBA32F        = 0x8814,

    R11F_G11F_B10F = 0x8C3A,


    RGB9_E5        = 0x8C3D,

    R8I            = 0x8231,
    R8UI           = 0x8232,

    R16I           = 0x8233,
    R16UI          = 0x8234,

    R32I           = 0x8235,
    R32UI          = 0x8236,

    RG8I           = 0x8237,
    RG8UI          = 0x8238,

    RG16I          = 0x8239,
    RG16UI         = 0x823A,

    RG32I          = 0x823B,
    RG32UI         = 0x823C,

    RGB8I          = 0x8D8F,
    RGB8UI         = 0x8D7D,

    RGB16I         = 0x8D89,
    RGB16UI        = 0x8D77,

    RGB32I         = 0x8D83,
    RGB32UI        = 0x8D71,


    RGBA8I         = 0x8D8E,
    RGBA8UI        = 0x8D7C,

    RGBA16I        = 0x8D88,
    RGBA16UI       = 0x8D76,

    RGBA32I        = 0x8D82,
    RGBA32UI       = 0x8D70,
};

enum class ImageUnitFormat : uint32_t {
    RGBA32F        = 0x8814,     //rgba32f
    RGBA16F        = 0x881A,     //rgba16f

    RG32F          = 0x8230,     //rg32f
    RG16F          = 0x822F,     //rg16f

    R11F_G11F_B10F = 0x8C3A,     //r11f_g11f_b10f

    R32F           = 0x822E,     //r32f
    R16F           = 0x822D,     //r16f

    RGBA32UI       = 0x8D70,     //rgba32ui
    RGBA16UI       = 0x8D76,     //rgba16ui

    RGB10_A2UI     = 0x906F,     //rgb10_a2ui

    RGBA8UI        = 0x8D7C,     //rgba8ui

    RG32UI         = 0x823C,     //rg32ui
    RG16UI         = 0x823A,     //rg16ui
    RG8UI          = 0x8238,     //rg8ui

    R32UI          = 0x8236,     //r32ui
    R16UI          = 0x8234,     //r16ui
    R8UI           = 0x8232,     //r8ui

    RGBA32I        = 0x8D82,     //rgba32i
    RGBA16I        = 0x8D88,     //rgba16i
    RGBA8I         = 0x8D8E,     //rgba8i

    RG32I          = 0x823B,     //rg32i
    RG16I          = 0x8239,     //rg16i
    RG8I           = 0x8237,     //rg8i

    R32I           = 0x8235,     //r32i
    R16I           = 0x8233,     //r16i
    R8I            = 0x8231,     //r8i

    RGBA16         = 0x805B,     //rgba16
    RGB10_A2       = 0x8059,     //rgb10_a2

    RGBA8          = 0x8058,     //rgba8

    RG16           = 0x822C,     //rg16
    RG8            = 0x822B,     //rg8

    R16            = 0x822A,     //r16
    R8             = 0x8229,     //r8

    RGBA16_SNORM   = 0x8F9B,     //rgba16_snorm
    RGBA8_SNORM    = 0x8F97,     //rgba8_snorm
    RG16_SNORM     = 0x8F99,     //rg16_snorm
    RG8_SNORM      = 0x8F95,     //rg8_snorm
    R16_SNORM      = 0x8F98,     //r16_snorm
    R8_SNORM       = 0x8F94,     //r8_snorm
};

}