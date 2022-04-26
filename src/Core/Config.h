#pragma once
#include "stdio.h"
#include <string>
#include <unordered_map>
#include <vector>

using SettingPair = std::unordered_map<std::string, std::string>;


class Config
{
public:
    Config(const char* path);
    ~Config();
    SettingPair parse_settings(bool* _end = nullptr, char* _name = nullptr, char* _type = nullptr);
    bool parse_pair(char* _key, char* _value); // true if success, false if failed
public:
    std::string get_msg() const { return msg; }
    bool good_;
private:
    std::string get_token();
    void error(const std::string& _msg) { good_ = false; msg = _msg; }
private:
    std::string msg;
    FILE* file;
    bool file_opened;
    const char* path;
};
