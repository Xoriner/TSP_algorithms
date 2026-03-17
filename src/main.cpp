#include <iostream>
#include <vector>
#include <chrono>

#include "tsp_bruteforce.h"
#include "utilities/utils.h"
#include "tsp_nn.h"

int main() {

    std::vector<std::vector<int>> graph_matrix = {
        {0,10,15,20},
        {10,0,35,25},
        {15,35,0,30},
        {20,25,30,0}
    };

    std::vector<std::vector<int>> graph_matrix_file = read_simple_input("instances/sym14.txt");

    //NEAREST NEIGHBOUR
    auto start_nn = std::chrono::high_resolution_clock::now();
    TSPResult result_nn = tsp_nearest_neighbor(graph_matrix_file, 0);
    auto stop_nn = std::chrono::high_resolution_clock::now();

    print_solution(result_nn);
    std::chrono::duration<double, std::milli> duration_nn = stop_nn - start_nn;
    std::cout << "NN Time: " << duration_nn.count() << " ms" << std::endl;

    //BRUTO FORCE
    auto start_bf = std::chrono::high_resolution_clock::now();
    TSPResult result_bf = tsp_bruteforce(graph_matrix_file);
    auto stop_bf = std::chrono::high_resolution_clock::now();

    //auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);
    //std::cout << duration.count() << " ns" << std::endl;

    // Zamiast duration_cast bo ucinało cześć ułamkową
    std::chrono::duration<double, std::milli> duration_bf = stop_bf - start_bf;
    std::cout << "BF Time: " << duration_bf.count() << " ms" << std::endl;

    print_solution(result_bf);

    return 0;
}