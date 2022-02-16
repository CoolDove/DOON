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
    std::vector<SettingPair> get_settings();
public:
    bool good_;
public:
    std::string get_token();
private:
    FILE* file;
    const char* path;
};

