#pragma once
#include "read_config.h"

std::map<std::string, std::string> read_config(const std::string& filename) {
    std::ifstream file(filename);
    std::map<std::string, std::string> config;
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string key, value;

        std::getline(ss, key, '=');
        std::getline(ss, value);

        // usuwanie spacji
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);

        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);

        config[key] = value;
    }

    return config;
}