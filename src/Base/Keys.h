#pragma once
#include <stdint.h>
#include "BitMaskEnum.h"

namespace Dove
{
enum class ModKey : uint32_t {
    None  = 0u,
    Shift = 1u<<1,
    Ctrl  = 1u<<2,
    Alt   = 1u<<3,
    Space = 1u<<4,
};BIT_MASK_ENUM(ModKey)

enum class KeyCode : uint32_t{
    None = 0u,
    A = 10u,
    B = 11u,
    C = 12u,
    D = 13u,
    E = 14u,
    F = 15u,
    G = 16u,
    H = 17u,
    I = 18u,
    J = 19u,
    K = 20u,
    L = 21u,
    M = 22u,
    N = 23u,
    O = 24u,
    P = 25u,
    Q = 26u,
    R = 27u,
    S = 28u,
    T = 29u,
    U = 30u,
    V = 31u,
    W = 32u,
    X = 33u,
    Y = 34u,
    Z = 35u,
    Space = 60u,
    Ctrl = 61u,
    Shift = 62u,
    Tab = 63u,
    Esc = 64u
};
}
