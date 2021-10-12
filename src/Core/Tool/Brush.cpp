#include "Brush.h"
#include "DoveLog.hpp"
#include "Core/Application.h"

namespace Tool
{
Brush::Brush(Application* _app) 
:   app_(_app)
{
    DLOG_TRACE("brush constructed");
}

Brush::~Brush() {

}
void Brush::on_init() {

}
void Brush::on_activate() {
    DLOG_TRACE("brush activated");
}
void Brush::on_deactivate() {

}

void Brush::on_mouse_down(Input::MouseInfo _info, Input::MouseButton _button, int _x, int _y) {
    Input::MouseInfo info = _info;
}
void Brush::on_mouse_up(Input::MouseInfo _info, Input::MouseButton _button, int _x, int _y) {

}

void Brush::on_mouse_move(Input::MouseInfo _info, int _x, int _y) {

}

void Brush::on_keyboard_down() {

}
void Brush::on_keyboard_up() {

}
void Brush::on_keyboard_move() {

}

void Brush::on_pen_down() {

}
void Brush::on_pen_up() {

}
void Brush::on_pen_move() {

}
}