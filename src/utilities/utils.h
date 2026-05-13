#pragma once

#include <vector>
#include <string>
#include <iostream>
#include "../algorithms/tsp_result.h"

std::vector<std::vector<int>> read_tsplib(const std::string& filename);
std::vector<std::vector<int>> read_simple_input(const std::string& filename);
void print_solution(const TSPResult& result);
std::string trim(const std::string& str);

inline double string_to_double(const std::string& s) {
    return std::stod(s);
}

inline int string_to_int(const std::string& s) {
    return std::stoi(s);
}