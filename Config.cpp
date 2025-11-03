#include "Config.h"
#include <fstream>
#include <iostream>

bool Config::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open config file '" << filename << "'\n";
        return false;
    }
    std::cout << "Config loaded successfully.\n";
    file.close();
    return true;
}