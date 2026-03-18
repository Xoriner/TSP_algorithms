#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

#include "tsp_result.h"

std::vector<std::vector<int>> read_simple_input(std::string filename);
std::vector<std::vector<int>> read_tsplib(std::string filename);
void print_solution(const TSPResult& result);

#endif //UTILS_H
