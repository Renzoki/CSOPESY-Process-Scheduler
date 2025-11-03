#pragma once
#include <string>

class Config {
public:
    static bool load(const std::string& filename);
};