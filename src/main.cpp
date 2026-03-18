#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>

#include "utilities/read_config.h"
#include "utilities/utils.h"
#include "algorithms/tsp_bruteforce.h"
#include "algorithms/tsp_rand.h"
#include "algorithms/tsp_nn.h"
#include "algorithms/tsp_rnn.h"

int main(int argc, char* argv[]) {

    std::string config_path = "config.txt";

    // jeśli podano argument, używamy go
    if (argc > 1) {
        config_path = argv[1];
    }

    // sprawdź czy plik istnieje
    std::ifstream test(config_path);
    if (!test) {
        std::cerr << "Error: Cannot open config file: " << config_path << std::endl;
        return 1;
    }

    auto config = read_config(config_path);

    // sprawdzenie wymaganych pól
    if (config.find("input_file") == config.end() ||
        config.find("algorithm") == config.end()) {
        std::cerr << "Error: Missing required config fields (input_file / algorithm)\n";
        return 1;
    }

    std::vector<std::vector<int>> matrix;

    if (config["input_file"].find(".tsp") != std::string::npos) {
        matrix = read_tsplib(config["input_file"]);
    } else {
        matrix = read_simple_input(config["input_file"]);
    }

    std::string algo = config["algorithm"];

    auto start = std::chrono::high_resolution_clock::now();
    TSPResult result;

    if (algo == "BF") {
        result = tsp_bruteforce(matrix);
    }
    else if (algo == "NN") {
        int start_node = config.count("start_node") ? std::stoi(config["start_node"]) : 0;
        result = tsp_nearest_neighbor(matrix, start_node);
    }
    else if (algo == "RNN") {
        result = tsp_rnn(matrix);
    }
    else if (algo == "RAND") {
        int runs = config.count("runs") ? std::stoi(config["runs"]) : 1000;
        result = tsp_rand(matrix, runs);
    }
    else {
        std::cerr << "Error: Unknown algorithm: " << algo << std::endl;
        return 1;
    }

    auto stop = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = stop - start;

    std::cout << "Time: " << duration.count() << " ms\n";

    if (config.count("output") && config["output"] == "1") {
        print_solution(result);
    }

    return 0;
}