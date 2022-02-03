#include "Serialize.h"
#include "DoveLog.hpp"

//-------------- reader -----------------//

DooReader::DooReader(const char* path)
:   good_(false)
{
    FILE* file = fopen(path, "rb");
    if (!file) return;

    header = read_header(file);

    read_layer_header(file);

    fclose(file);
    good_ = true;
    path_ = path;
}

DooHeader DooReader::read_header(FILE* file) {
    DooHeader header;
    fread(&header, sizeof(DooHeader), 1, file);
    return header;
}

void DooReader::read_layer_header(FILE* file) {
    fseek(file, DOO_HEADER_SIZE, SEEK_SET);

    Col_RGBA* container = (Col_RGBA*)malloc(header.width * header.height * sizeof(Col_RGBA));
    while (true) {
        fpos_t start_pos;
        fgetpos(file, &start_pos);

        LayerHeader lheader;

        char* name_buf = (char*)malloc(sizeof(char) * DOO_MAX_NAME_LENGTH);

        int count = fread(&lheader, sizeof(LayerHeader), 1, file);// read layer header
        if (count < 1) break;
        count = fread(name_buf, sizeof(char), DOO_MAX_NAME_LENGTH, file);// read layer name
        if (count < DOO_MAX_NAME_LENGTH) break;

        std::string name(name_buf);

        fpos_t pos;
        fgetpos(file, &pos);

        layer_headers.push_back(lheader);
        layer_names.push_back(name);
        layer_offsets.push_back(pos);

        fseek(file, (long)lheader.data_size_b, SEEK_CUR);
    }
    free(container);
}


//-------------- writer -----------------//

DooWriter::DooWriter(Scene* scene)
:   scene_(scene),
    header{ 0 }
{
}

DooHeader DooWriter::make_header(Scene* scene) {
    header.version = DOO_VERSION;
    header.width = scene->info_.width;
    header.height = scene->info_.height;
    return header;
}

static inline int min(int a, int b) {
    return a < b ? a : b;
}


LayerHeader DooWriter::make_layer_header(Layer* layer) {
    LayerHeader lheader;
    lheader.blend_mode = (uint32_t)(layer->info_.blend_mode);
    lheader.data_size_b = layer->data_size();
    return lheader;
}

static inline size_t get_layer_name(Layer* layer, char* buf, size_t bufsize) {
    memset(buf, 0x0, bufsize);
    size_t length = min(bufsize, layer->info_.name.length());
    for (int i = 0; i < length; i++) {
        buf[i] = layer->info_.name[i];
    }
    return length;
}

void DooWriter::write(const char* path) {
    make_header(scene_);
    FILE* file = fopen(path, "wb");

    fwrite(&header, sizeof(DooHeader), 1, file);// write doo header

    char* name_buf = (char*)malloc(DOO_MAX_NAME_LENGTH * sizeof(char));
    for (auto layer = scene_->layers_.begin(); layer != scene_->layers_.end(); layer++) {
        auto l = (*layer).get();
        LayerHeader layerh = make_layer_header(l);

        size_t name_length = get_layer_name(l, name_buf, DOO_MAX_NAME_LENGTH);

        fwrite(&layerh, sizeof(LayerHeader), 1, file);// write layer header

        fwrite(name_buf, sizeof(char), DOO_MAX_NAME_LENGTH, file);// write layer name

        (*layer)->mem_fetch();
        if (!(*layer)->pixels_) {
            DLOG_ERROR("failed to fetch layer: %s", (*layer)->info_.name.c_str());
            free(name_buf);
            fclose(file);
            return;
        }

        size_t data_count = header.width * header.height;
        size_t write = fwrite((*layer)->pixels_, sizeof(Col_RGBA), data_count, file);// write layer data

        (*layer)->mem_release();
    }
    
    free(name_buf);
    fclose(file);

    DLOG_DEBUG("saved to %s", path);
}
