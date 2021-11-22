#include "Input.h"
#include <Core/Application.h>
#include <Core/Tool/Brush.h>
#include <DoveLog.hpp>
#include <imgui/imgui.h>
#include <Core/Action.h>
#include <Core/Renderer.h>

#define IMGUI_MONOPOLY_INPUT 0x0010
namespace Input {
InputProcess imgui_proc = nullptr;
InputContext input_context = {0};

// FIXME: in some unknown situation, brush cannot paint

// TODO: mouse scroll to zoom the camera, middle drag to move the camera
// TODO: alt-mouse_right drag to adjust the brush size
        // currently, ctrl scroll to adjust the brush size
// TODO: load key-action map from the config file


// check if mouse is in region for tools
bool is_mouse_position_valid(Application* _app, int _x, int _y) {
    return !(_x >= _app->window_info_.width || _x <= 0 ||
             _y >= _app->window_info_.height|| _y <= 0);
}

void get_pen_position_from_lparam(Application* app, LPARAM _lparam, int* _x, int* _y) {
    *_x = LOWORD(_lparam) - app->window_info_.posx;
    *_y = HIWORD(_lparam) - app->window_info_.posy;
}

void release_mouse_and_pen(Application* _app, int _mx, int _my) {
    PointerInfo info = {0};
    if (input_context.mouse_down_l) {
        input_context.mouse_down_l = false;
        info.button = Input::PointerButton::LEFT;
        _app->curr_tool_->on_pointer_up(info, _mx, _my);
    }
    if (input_context.mouse_down_r) {
        input_context.mouse_down_r = false;
        info.button = Input::PointerButton::LEFT;
        _app->curr_tool_->on_pointer_up(info, _mx, _my);
    }
    if (input_context.mouse_down_m) {
        input_context.mouse_down_m = false;
        info.button = Input::PointerButton::LEFT;
        _app->curr_tool_->on_pointer_up(info, _mx, _my);
    }
    if (input_context.pen_down) {
        input_context.pen_down = false;
        info.button = Input::PointerButton::PEN;
        _app->curr_tool_->on_pointer_up(info, _mx, _my);
    }
}

uint32_t on_key_down(KeyCode _key) {
    switch (_key)
    {
    case VK_MENU:
        input_context.mod_key |= ModKey::Alt;
        break;
    case VK_CONTROL:
        input_context.mod_key |= ModKey::Ctrl;
        break;
    case VK_SHIFT:
        input_context.mod_key |= ModKey::Shift;
        break;
    case VK_SPACE:
        input_context.mod_key |= ModKey::Space;
        break;
    default:
        break;
    }

    if (_key >= 0x41 && _key <= 0x5A) {
        // is letter key, from A to Z
        Dove::KeyCode key = static_cast<Dove::KeyCode>(_key - (0x41 - 10u));
        Application* app = Application::instance_;
        ActionKey action_key;
        action_key.key = key;
        action_key.mod = input_context.mod_key;
        app->action_list_->invoke(action_key);
    }

    return 1;
}

uint32_t on_key_repeat(KeyCode _key) {
    return 1;
}

uint32_t on_key_up(KeyCode _key) {
    switch (_key)
    {
    case VK_MENU:
        input_context.mod_key &= ~ModKey::Alt;
        break;
    case VK_CONTROL:
        input_context.mod_key &= ~ModKey::Ctrl;
        break;
    case VK_SHIFT:
        input_context.mod_key &= ~ModKey::Shift;
        break;
    case VK_SPACE:
        input_context.mod_key &= ~ModKey::Space;
        break;
    default:
        break;
    }
    return 1;
}

// NOTE: |[Mouse Message Rules For Tools]|
// when mouse position is in tool region, clicking will set the state in 
// input_context no matter whether imgui is capturing mouse.
// when you release mouse(imgui capturing or not), or move out tool region,
// a pointer_up() is called, and the corresponding value in input_context will be set
// [ button down ] should be blocked by imgui, but [ button up ] should go through
// ImGui::GetIO().WantCaptureMouse would be set to true only when you button down in the imgui region
// so, it's not necessary to check this when processing button up messages

LRESULT CALLBACK wnd_proc(HWND _window, UINT _message, WPARAM _wparam, LPARAM _lparam) {
    if (imgui_proc) {
        LRESULT rst = imgui_proc(_window, _message, _wparam, _lparam);
        if (rst == IMGUI_MONOPOLY_INPUT) {
            return 1;
        }
    }
    
    static POINTER_PEN_INFO pen_info{};

    Application* app = Application::instance_;

    LRESULT result = 0;
    switch (_message)
    {
        case WM_SIZE:
        {
            app->window_info_.width = LOWORD(_lparam);
            app->window_info_.height = HIWORD(_lparam);

            int width  = app->window_info_.width;
            int height = app->window_info_.height;

            // TODO: move to somewhere else
            if (app->inited_) {
                glViewport(0, 0, width, height);
                app->renderer_->resize_framebuffer({width, height});
            }

        } break;
        case WM_MOVE:
        {
            app->window_info_.posx = LOWORD(_lparam);
            app->window_info_.posy = HIWORD(_lparam);
        } break;
        case WM_DESTROY:
        {
            PostQuitMessage(0);
        } break;
        case WM_CLOSE:
        {
            PostQuitMessage(0);
        } break;
        case WM_ACTIVATEAPP:
        {
            DLOG_TRACE("activate\n");
        } break;
        case WM_KEYDOWN:
        {
            bool repeat = 1u<<30 & _lparam;

            if (repeat) on_key_repeat(_wparam);
            else        on_key_down(_wparam);
            
        } break;
        case WM_KEYUP:
        {
            on_key_up(_wparam);
        } break;
        case WM_LBUTTONDOWN:
        {
            if (!app->curr_tool_) break;

            int mx = LOWORD(_lparam);
            int my = HIWORD(_lparam);

            if (is_mouse_position_valid(app, mx, my)) {
                input_context.mouse_down_l = true;

                if (!ImGui::GetIO().WantCaptureMouse) {
                    pen_info = {0};
                    pen_info.pressure = 1;
                    PointerInfo info = {0};
                    info.pen_info = pen_info;
                    info.button = Input::PointerButton::LEFT;
                    Input::get_btnstate_from_wparam(_wparam, &info.btn_state);
                    app->curr_tool_->on_pointer_down(info, mx, my);
                }
            }
        } break;
        case WM_LBUTTONUP:
        {
            if (!app->curr_tool_) break;

            int mx = LOWORD(_lparam);
            int my = HIWORD(_lparam);
            if (is_mouse_position_valid(app, mx, my) || input_context.mouse_down_l) {
                input_context.mouse_down_l = false;

                pen_info = {0};
                PointerInfo info = {0};
                info.pen_info = pen_info;
                info.button = Input::PointerButton::LEFT;
                Input::get_btnstate_from_wparam(_wparam, &info.btn_state);
                app->curr_tool_->on_pointer_up(info, mx, my);
            }
        } break;
        // TODO: mouse region limitaions for mouse button r m and pointer
        case WM_RBUTTONDOWN:
        {
            if (!app->curr_tool_) break;

            int mx = LOWORD(_lparam);
            int my = HIWORD(_lparam);
            if (is_mouse_position_valid(app, mx, my)) {
                input_context.mouse_down_r = true;
                if (!ImGui::GetIO().WantCaptureMouse) {
                    pen_info = {0};
                    PointerInfo info = {0};
                    info.pen_info = pen_info;
                    info.button = Input::PointerButton::RIGHT;
                    Input::get_btnstate_from_wparam(_wparam, &info.btn_state);
                    app->curr_tool_->on_pointer_down(info, mx, my);
                }
            }
        } break;
        case WM_RBUTTONUP:
        {
            if (!app->curr_tool_) break;

            int mx = LOWORD(_lparam);
            int my = HIWORD(_lparam);
            if (is_mouse_position_valid(app, mx, my) || input_context.mouse_down_r) {
                input_context.mouse_down_r = false;

                pen_info = {0};
                PointerInfo info = {0};
                info.pen_info = pen_info;
                info.button = Input::PointerButton::RIGHT;
                Input::get_btnstate_from_wparam(_wparam, &info.btn_state);
                app->curr_tool_->on_pointer_up(info, LOWORD(_lparam), HIWORD(_lparam));
            }
        } break;
        case WM_MBUTTONDOWN:
        {
            if (!app->curr_tool_) break;

            int mx = LOWORD(_lparam);
            int my = HIWORD(_lparam);
            if (is_mouse_position_valid(app, mx, my) || input_context.mouse_down_m) {
                input_context.mouse_down_m = true;

                if (!ImGui::GetIO().WantCaptureMouse) {
                    pen_info = {0};
                    PointerInfo info = {0};
                    info.pen_info = pen_info;
                    info.button = Input::PointerButton::MIDDLE;
                    Input::get_btnstate_from_wparam(_wparam, &info.btn_state);
                    app->curr_tool_->on_pointer_down(info, LOWORD(_lparam), HIWORD(_lparam));
                }
            }
        } break;
        case WM_MBUTTONUP:
        {
            if (!app->curr_tool_) break;

            int mx = LOWORD(_lparam);
            int my = HIWORD(_lparam);
            if (is_mouse_position_valid(app, mx, my) || input_context.mouse_down_m) {
                input_context.mouse_down_m = false;

                pen_info = {0};
                PointerInfo info = {0};
                info.pen_info = pen_info;
                info.button = Input::PointerButton::MIDDLE;
                Input::get_btnstate_from_wparam(_wparam, &info.btn_state);
                app->curr_tool_->on_pointer_up(info, LOWORD(_lparam), HIWORD(_lparam));
            }
        }
        case WM_MOUSEMOVE:
        {
            if (!app->curr_tool_) break;

            int mx = LOWORD(_lparam);
            int my = HIWORD(_lparam);

            // valid, invoke on_pointer
            if (is_mouse_position_valid(app, mx, my)) {
                if (!ImGui::GetIO().WantCaptureMouse && !input_context.pen_down) {
                    pen_info = {0};
                    PointerInfo info = {0};
                    info.pen_info = pen_info;
                    info.button = Input::PointerButton::OTHER;
                    Input::get_btnstate_from_wparam(_wparam, &info.btn_state);
                    
                    app->curr_tool_->on_pointer(info, LOWORD(_lparam), HIWORD(_lparam));
                }
            } else {
                // when mouse moves outside the tool region
                // input_context is updated and if any mouse button is holding
                // corresponding pointer_up() would be called
                if (!app->curr_tool_) break;
                release_mouse_and_pen(app, mx, my);
            }

            input_context.mouse_pos.x = LOWORD(_lparam);
            input_context.mouse_pos.y = HIWORD(_lparam);
            
        } break;
        case WM_POINTERDOWN:
        {
            DLOG_TRACE("pointer down");
            if (!app->curr_tool_) break;

            int mx, my;
            get_pen_position_from_lparam(app, _lparam, &mx, &my);

            if (is_mouse_position_valid(app, mx, my)) {
                input_context.pen_down = true;

                if (!ImGui::GetIO().WantCaptureMouse) {
                    PointerInfo info = {0};
                    GetPointerPenInfo(GET_POINTERID_WPARAM(_wparam), &pen_info);
                    info.pen_info = pen_info;
                    info.button = Input::PointerButton::PEN;
                    info.btn_state = {0};
                    app->curr_tool_->on_pointer_down(info, mx, my);
                }
            }
            SetCursorPos(LOWORD(_lparam), HIWORD(_lparam));
        } break;
        case WM_POINTERUPDATE:
        {
            if (!app->curr_tool_) break;

            int mx, my;
            get_pen_position_from_lparam(app, _lparam, &mx, &my);

            // valid, invoke on_pointer
            if (is_mouse_position_valid(app, mx, my)) {
                if (!ImGui::GetIO().WantCaptureMouse) {
                    PointerInfo info = {0};
                    GetPointerPenInfo(GET_POINTERID_WPARAM(_wparam), &pen_info);
                    info.pen_info = pen_info;
                    info.button = Input::PointerButton::PEN;
                    info.btn_state = {0};

                    app->curr_tool_->on_pointer(info, mx, my);
                }
            } else if (app->curr_tool_) {
                // when mouse move outside the tool region
                // if any mouse button is holding,
                // corresponding pointer_up() would be called
                release_mouse_and_pen(app, mx, my);
            }
            input_context.mouse_pos.x = mx;
            input_context.mouse_pos.y = my;

            if (!input_context.pen_down) SetCursorPos(LOWORD(_lparam), HIWORD(_lparam));

        } break;
        case WM_POINTERUP:
        {
            if (!app->curr_tool_) break;

            int mx, my;
            get_pen_position_from_lparam(app, _lparam, &mx, &my);

            input_context.pen_down = false;
            PointerInfo info = {0};
            GetPointerPenInfo(GET_POINTERID_WPARAM(_wparam), &pen_info);
            info.pen_info = pen_info;
            info.button = Input::PointerButton::PEN;
            info.btn_state = {0};
            app->curr_tool_->on_pointer_up(info, mx, my);
            SetCursorPos(LOWORD(_lparam), HIWORD(_lparam));
        } break;
        case WM_MOUSELEAVE:
        {
            // DLOG_TRACE("mouse leaved");
        } break;
        case WM_MOUSEWHEEL:
        {
            // @TEMPCODE:
            // these lines are very temporary, to adjust camera size,
            // should invoke some camera-member-function
            // like- cam->adjust_size(fac);
            int wd = GET_WHEEL_DELTA_WPARAM(_wparam) / WHEEL_DELTA;
            if (wd > 1) wd = 1;
            if (wd < -1) wd = -1;

            if (!get_uint(input_context.mod_key & ModKey::Ctrl)) {
                // @AdjustCameraSize:
                DGL::Camera* cam = &app->curr_scene_->camera_;
                float size_delta = (float)wd * 0.1f;
                float cam_size_max = 9.0f;
                float cam_size_min = 0.1f;
                if (cam->size_ + size_delta <= cam_size_max) {
                    cam->size_ += size_delta;
                } else {
                    cam->size_ = cam_size_max;
                }
                if (cam->size_ + size_delta >= cam_size_min) {
                    cam->size_ += size_delta;
                } else {
                    cam->size_ = cam_size_min;
                }
            } else {
                // @AdjustBrushSize:
                Tool::Brush* brush = dynamic_cast<Tool::Brush*>(app->curr_tool_);
                uint32_t spd = 3;
                if (brush) {
                    if (brush->size_max_ + wd * spd >= 7500) {
                        brush->size_max_ = 7500;
                    } else if (brush->size_max_ + wd * spd <= 1) {
                        brush->size_max_ = 1;
                    } else {
                        brush->size_max_ += wd;
                    }
                }

            }
        } break;
        default:
        {
            result = DefWindowProc(_window, _message, _wparam, _lparam);
        } break;
    }

    if (app->inited_) {
        ImGuiIO& io = ImGui::GetIO();
        io.MousePos.x = (float)input_context.mouse_pos.x;
        io.MousePos.y = (float)input_context.mouse_pos.y;
        io.MouseDown[0] = input_context.mouse_down_l | input_context.pen_down;
        io.MouseDown[1] = input_context.mouse_down_r;
        io.MouseDown[3] = input_context.mouse_down_m;
    }
    return result;
}

void get_btnstate_from_wparam(WPARAM _wparam, ButtonState* _state) {
    _state->mouse_l = _wparam & MK_LBUTTON;
    _state->mouse_r = _wparam & MK_RBUTTON;
    _state->mouse_m = _wparam & MK_MBUTTON;
}

}
