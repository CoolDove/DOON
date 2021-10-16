#pragma once
#include <string>

class Layer {
public:
    Layer();
    ~Layer();
public:
    Image content_;

    struct {
        std::string name;
    } info_;

};