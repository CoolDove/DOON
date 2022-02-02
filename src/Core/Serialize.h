#pragma once
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <array>
#include <Core/Scene.h>
// #include <stb_image/std_image.h>

const int DOO_VERSION = 1;

inline void ReadDoo(const char* path) {
    FILE* file = fopen(path, "r");

    char* data = (char*)malloc(sizeof(char) * 512);
    fread(data, sizeof(char), 512, file);

    fclose(file);
};

struct LayerHeader
{
    uint32_t blend_mode;
    uint64_t data_size;
    std::array<char, 256> name;
};

struct DooHeader
{
    uint32_t version;
    uint32_t width;
    uint32_t height;
};

class DooReader
{
public:
    DooReader(const char* path);
public:
    DooHeader header;
    std::vector<LayerHeader> layer_headers;
private:
    DooHeader   read_header(FILE* file);
    LayerHeader read_layer_header(FILE* file);
};

class DooWriter
{
public:
    DooWriter(Scene* scene);
    void write(const char* path);
public:
    DooHeader header;
    std::vector<LayerHeader> layer_headers;
private:
    DooHeader make_header(Scene* scene);
    LayerHeader make_layer_header(Layer* layer);
private:
    Scene* scene_;
};
