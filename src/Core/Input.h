#include <windows.h>

// #define MK_CONTROL  0x0008
// #define MK_LBUTTON  0x0001
// #define MK_MBUTTON  0x0010
// #define MK_RBUTTON  0x0002
// #define MK_SHIFT    0x0004
// #define MK_XBUTTON1 0x0020
// #define MK_XBUTTON2 0x0040

namespace Input{
using bool8       = char;
using WPARAM      = unsigned __int64;
// using PointerInfo = POINTER_PEN_INFO;

enum class PointerButton : unsigned char {
    OTHER   = 0,
    LEFT    = 1,
    RIGHT   = 2,
    MIDDLE  = 3,
    PEN     = 4,
};

struct PointerInfo {
    POINTER_PEN_INFO pen_info;
    PointerButton button;
};
}