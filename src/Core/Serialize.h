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
    FILE* file = fopen(path, "rb");

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

const uint32_t DOO_HEADER_SIZE = sizeof(DooHeader);
const uint32_t DOO_LAYER_HEADER_SIZE = sizeof(LayerHeader) + sizeof(char) * DOO_MAX_NAME_LENGTH;

class DooReader
{
public:
    DooReader(const char* path);
public:
    DooHeader header;
    std::vector<LayerHeader> layer_headers;
    std::vector<std::string> layer_names;
    std::vector<fpos_t> layer_offsets;
    bool good() const { return good_; }

    uint32_t get_layers_count() const { return (uint32_t)layer_headers.size(); }
    LayerHeader get_layer_header(uint32_t layer) {
        if (layer >= get_layers_count()) return {0};
        return layer_headers[layer];
    }
    std::string get_layer_name(uint32_t layer) {
        if (layer >= get_layers_count()) return "";
        return layer_names[layer];
    }
    uint64_t get_layer_data_offset(uint32_t layer) {
        if (layer >= get_layers_count()) return 0;
        return layer_offsets[layer];
    }
    void get_layer_pixels(uint32_t layer, Col_RGBA* buffer) {
        if (layer >= get_layers_count()) return;

        // TODO: decode here.
        FILE* file = fopen(path_.c_str(), "rb");
        fseek(file, get_layer_data_offset(layer), SEEK_SET);
        fread(buffer, sizeof(Col_RGBA), header.width * header.height, file);
        fclose(file);
    }
private:
    bool good_;
    std::string path_;
    DooHeader read_header(FILE* file);
    void      read_layer_header(FILE* file);
};

class DooWriter
{
public:
    DooWriter(Scene* scene);
    void write(const char* path);
public:
    DooHeader header;
    // std::vector<LayerHeader> layer_headers;
private:
    DooHeader make_header(Scene* scene);
    LayerHeader make_layer_header(Layer* layer);
private:
    Scene* scene_;
};
