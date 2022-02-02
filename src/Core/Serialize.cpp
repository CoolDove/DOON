#include "Serialize.h"

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

DooWriter::DooWriter(Scene* scene)
:   scene_(scene)
{
    // set up headers
    header.version = 0.1f;
    header.width = scene->info_.width;
    header.height = scene->info_.height;
}

DooHeader DooWriter::make_header(Scene* scene) {
    header.version = DOO_VERSION;
    header.width = scene->info_.width;
    header.height = scene->info_.height;
    return header;
}

LayerHeader DooWriter::make_layer_header(Layer* layer) {
    LayerHeader lheader;
    return lheader;
}

void DooWriter::write(const char* path) {
    FILE* file = fopen(path, "w");
    fwrite(&header, sizeof(header), 1, file);
    fclose(file);
}
