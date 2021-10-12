#define MK_CONTROL  0x0008
#define MK_LBUTTON  0x0001
#define MK_MBUTTON  0x0010
#define MK_RBUTTON  0x0002
#define MK_SHIFT    0x0004
#define MK_XBUTTON1 0x0020
#define MK_XBUTTON2 0x0040

namespace Input{
using bool8 = char;
using WPARAM = unsigned __int64;

enum class MouseButton : unsigned char {
    OTHER   = 0,
    LEFT    = 1,
    RIGHT   = 2,
    MIDDLE  = 3,
};

struct MouseInfo {
    bool8 control;
    bool8 shift;

    bool8 mouse_l;
    bool8 mouse_r;
    bool8 mouse_m;
     
    bool8 mouse_x1;
    bool8 mouse_x2;
};

inline void parse_mouse_info_from_wparam(WPARAM _w, MouseInfo* _info) {
    if (!_info) return;
    _info->control  = _w & MK_CONTROL;
    _info->shift    = _w & MK_SHIFT;

    _info->mouse_l  = _w & MK_LBUTTON;
    _info->mouse_r  = _w & MK_RBUTTON;
    _info->mouse_m  = _w & MK_MBUTTON;

    _info->mouse_x1 = _w & MK_XBUTTON1;
    _info->mouse_x2 = _w & MK_XBUTTON2;
}

}