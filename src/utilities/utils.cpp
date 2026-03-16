#include "utils.h"

#include <fstream>
#include <iostream>

#include "tsp_result.h"

std::vector<std::vector<int>> read_simple_input(std::string filename) {
    std::ifstream input_file(filename);

    if (!input_file.is_open()) {
        //cerr instant error
        std::cerr << "Error opening file " << filename << std::endl;
    }

    //TODO
    //make error handling

    int n;
    input_file >> n;

    std::vector<std::vector<int>> graph_matrix(n, std::vector<int>(n));

    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            input_file >> graph_matrix[i][j];
        }
    }

    return graph_matrix;

}

void print_solution(TSPResult result) {
    std::cout << "Cost: " << result.cost << "\n";

    std::cout << "Path: 0";
    for (int v : result.path)
        std::cout << "-> " << v;
    std::cout << "-> 0\n";
}
