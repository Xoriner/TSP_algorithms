#pragma once

#include <fstream>
#include <map>
#include <string>
#include <iostream>

/**
 * Parser konfiguracji tolerancyjny na:
 * - Spacje wokół '='
 * - Komentarze po wartości (# ...)
 * - Puste linie
 * - Dowolne formatowanie
 */
inline std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";

    size_t end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::map<std::string, std::string> read_config(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open config file: " << filename << std::endl;
        exit(1);
    }

    std::map<std::string, std::string> config;
    std::string line;

    while (std::getline(file, line)) {
        // Usuń komentarz (wszystko po '#')
        size_t hash_pos = line.find('#');
        if (hash_pos != std::string::npos) {
            line = line.substr(0, hash_pos);
        }

        // Trimuj spacje
        line = trim(line);

        // Pusta linia - skip
        if (line.empty()) {
            continue;
        }

        // Szukaj znaku '='
        size_t eq_pos = line.find('=');
        if (eq_pos == std::string::npos) {
            continue;  // Brak '=' - skip
        }

        // Rozdziel na klucz i wartość
        std::string key = line.substr(0, eq_pos);
        std::string value = line.substr(eq_pos + 1);

        // Trimuj obie strony
        key = trim(key);
        value = trim(value);

        // Zapisz jeśli obie nie-puste
        if (!key.empty() && !value.empty()) {
            config[key] = value;
        }
    }

    file.close();
    return config;
}