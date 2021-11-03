#include "Compositor.h"

Compositor::Compositor() {
    comp_shaders_["default"].init();
}

Compositor::~Compositor() {
}

void Compositor::add_compshader(std::string _name, std::string _load_path) {
}

void Compositor::compose() {
}
