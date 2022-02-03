#pragma once
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <array>
#include <Core/Scene.h>
// #include <stb_image/std_image.h>

const uint32_t DOO_VERSION = 0xacbcdcec;
const uint64_t DOO_MAX_NAME_LENGTH = 256;


/*
.doo
| doo header | layer0 header | layer0 name | layer0 data| layer1 header | layer1 name | layer1 data|...
|------------|--------------- ------------- ------------|--------------- ------------- ------------|-----

doo header: sizeof DooHeader
layer header: sizeof LayerHeader
name: char[DOO_MAX_NAME_LENGTH]
*/

inline void ReadDoo(const char* path) {
    FILE* file = fopen(path, "r");

    char* data = (char*)malloc(sizeof(char) * 512);
    fread(data, sizeof(char), 512, file);

    fclose(file);
};

struct LayerHeader
{
    uint32_t blend_mode;
    size_t data_size_b;
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
