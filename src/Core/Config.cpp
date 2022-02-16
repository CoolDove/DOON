#include "Config.h"
#include "DoveLog.hpp"

Config::Config(const char* path) {
    file = fopen(path, "r");
    if (!file) {
        DLOG_ERROR("failed to find config file: %s", path);
        return;
    }
    good_ = true;
    this->path = path;
}

Config::~Config() {
    if (good_) fclose(file);
}


static bool valid_name_char(char c) {
    return (c >= '0' && c <= '9') ||
           (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') ||
           (c == '_' || c == '-' || c == '<' || c == '>' || c == '.');
}

std::string Config::get_token() {
    int next = ' ';
    while (next == ' ' || next == '\n' ) {
        next = fgetc(file);
    }
    
    if (next == EOF) return "";

    if (next == ':') return ":";
    if (next == '{') return "{";
    if (next == '}') return "}";
    if (next == ',') return ",";

    std::string token;
    while (valid_name_char((char)next)) {
        token += (char)next;
        next = fgetc(file);
        if (next == EOF) {
            return token;
        }
    }
    fseek(file, -1, SEEK_CUR);
    return token;
}
