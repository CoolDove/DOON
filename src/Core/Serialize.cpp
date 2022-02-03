#include "Serialize.h"
#include "DoveLog.hpp"

DooReader::DooReader(const char* path) {
    FILE* file = fopen(path, "r");
    if (!file) return;

    header = read_header(file);

    fclose(file);
}

DooHeader DooReader::read_header(FILE* file) {
    DooHeader header;
    fread(&header, sizeof(DooHeader), 1, file);
    return header;
}

LayerHeader DooReader::read_layer_header(FILE* file) {
    LayerHeader header = {0};
    return header;
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
    FILE* file = fopen(path, "w");

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

        fwrite((*layer)->pixels_, sizeof(char), layerh.data_size_b, file);// write layer data

        (*layer)->mem_release();
    }
    
    free(name_buf);
    fclose(file);

    DLOG_DEBUG("saved to %s", path);
}
