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
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    Space,
    Ctrl,
    Shift,
    Tab,
    Esc
};
}