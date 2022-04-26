#include "Config.h"
#include "DoveLog.hpp"

Config::Config(const char* path) {
    file = fopen(path, "r");
    if (!file) {
        error("failed to find config file: %s");
        return;
    }
    file_opened = true;
    good_ = true;
    this->path = path;
}

Config::~Config() {
    if (file_opened) fclose(file);
}

static bool valid_name_char(char c) {
    return (c >= '0' && c <= '9') ||
           (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') ||
           (c == '_' || c == '-' || c == '<' || c == '>' || c == '.');
}

std::string Config::get_token() {
    int next = ' ';
    while (next == ' ' || next == '\n' || next == '\t') {
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


bool Config::parse_pair(char* _key, char* _value) {
    std::string token = get_token();
    std::string key_stash = token;

    token = get_token();
    if (token == ":") {
        token = get_token();
        strcpy(_key, key_stash.c_str());
        strcpy(_value, token.c_str());
        return true;
    } else {
        return false;
    }
}

SettingPair Config::parse_settings(bool* _end, char* _name, char* _type) {
    SettingPair empty;

    if (!good_) {
        if (_end) *_end = true;
        return empty;
    }

    SettingPair output;
    std::string type;
    std::string name;

    std::string token_stash = get_token();
    type = token_stash;
    if (token_stash == "keymap" ||
        token_stash == "brush" ||
        token_stash == "blendmode")
    {
        token_stash = get_token();
        if (token_stash != ":") {
            error("config error: missing ':'");
            return empty;
        }

        name = get_token();

        token_stash = get_token();
        if (token_stash != "{") {
            error("config error: missing '{'");
            return empty;
        }

        // parse setting pairs
        char* key = (char*)malloc(256 * sizeof(char)) ;
        char* value = (char*)malloc(256 * sizeof(char));

        bool pair_result = true;
        while (pair_result) {
            pair_result = parse_pair(key, value);
            if (!pair_result) {
                error("failed to parse pair");
                return empty;
            }
            output[key] = value;
            
            token_stash = get_token();

            if (token_stash == "}") {
                if (_name != nullptr) strcpy(_name, name.c_str());
                if (_type != nullptr) strcpy(_type, type.c_str());
                return output;
            } else if (token_stash == ",") {
                // nothing
            } else {
                error("failed");
                return empty;
            }
        }
        free(key);
        free(value);
        
        token_stash = get_token();
        if (token_stash != "}") {
            if (_name != nullptr) strcpy(_name, name.c_str());
            if (_type != nullptr) strcpy(_type, type.c_str());
            return output;
        }
    } else if (token_stash == "") {// EOF
        if (_end) *_end = true;
        return empty;
    } else {
        error("settings type error, only support: keymap, brush, blendmode");
        return empty;
    }
}
