#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <limits>
#include <string>
#include <iomanip>

#include "utilities/read_config.h"
#include "utilities/utils.h"
#include "algorithms/tsp_branch_and_bound.h" //Nagłówek dla Branch and Bound
#include "algorithms/tsp_rnn.h" // Do wyznaczenia wstępnego UB

int main(int argc, char* argv[]) {
    std::string config_path = "config.txt";
    if (argc > 1) config_path = argv[1];

    std::ifstream test(config_path);
    if (!test) {
        std::cerr << "Error: Cannot open config file: " << config_path << std::endl;
        return 1;
    }

    auto config = read_config(config_path);

    // Wczytywanie instancji z pliku i search_mode wybranego
    if (config.find("input_file") == config.end() || config.find("search_mode") == config.end()) {
        std::cerr << "Error: Missing required fields (input_file / search_mode)\n";
        return 1;
    }

    std::vector<std::vector<int>> matrix;
    std::string filename = config["input_file"];

    // Sprawdzamy czy plik to format TSPLIB (.tsp lub .atsp)
    if (filename.find(".tsp") != std::string::npos || filename.find(".atsp") != std::string::npos) {
    matrix = read_tsplib(filename);
    } else {
    // To wywoła się tylko dla Twoich własnych plików .txt,
    // które mają tylko rozmiar i macierz
    matrix = read_simple_input(filename);
    }

    if (matrix.empty()) return 1;

    // Pobranie parametrów specyficznych dla B&B
    // search_mode: 1=BFS, 2=DFS, 3=Best-First
    int search_mode = std::stoi(config["search_mode"]);
    bool use_initial_ub = (config.count("use_initial_ub") && config["use_initial_ub"] == "1");

    int initial_ub = std::numeric_limits<int>::max();

    // 1. Opcjonalne wyznaczenie UB za pomocą RNN
    if (use_initial_ub) {
        TSPResult rnn_res = tsp_rnn(matrix);
        initial_ub = rnn_res.cost;
        std::cout << "Initial UB (from RNN): " << initial_ub << "\n";
    }

    std::cout << "Starting Branch-and-Bound (Mode: " << search_mode << ")..." << std::endl;

    // 2. Pomiar czasu właściwego algorytmu B&B
    auto start_time = std::chrono::high_resolution_clock::now();

    // Twoja funkcja B&B powinna przyjmować macierz, tryb i opcjonalne UB
    TSPResult result = tsp_branch_and_bound(matrix, search_mode, initial_ub);

    auto stop_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration_ms = stop_time - start_time;

    // Wyświetlanie wyników
    std::cout << "\n================ B&B RESULT ================\n";
    std::cout << "Search Mode:    " << search_mode << "\n";
    std::cout << "Best Cost:      " << result.cost << "\n";
    //std::cout << "Nodes Visited:  " << result.nodes_visited << "\n"; // Warto liczyć wierzchołki do raportu
    std::cout << "Total Time:     " << duration_ms.count() << " ms\n";
    std::cout << "============================================\n";

    if (config.count("output") && config["output"] == "1") {
        std::cout << "Best Path: ";
        print_solution(result);
    }

    // ======== Zapis do CSV (wyniki_bab.csv) ========
    std::ofstream csv_file("results_bab.csv", std::ios::app);
    if (csv_file.is_open()) {
        csv_file.seekp(0, std::ios::end);
        if (csv_file.tellp() == 0) {
            // Nagłówek: Typ, Plik, Rozmiar, Tryb, Czy_z_UB, Koszt, Czas_ms
            csv_file << "File,N,Mode,InitUB,Cost,Time_ms\n";
        }

        csv_file << config["input_file"] << ","
                 << matrix.size() << ","
                 << search_mode << ","
                 << (use_initial_ub ? "YES" : "NO") << ","
                 << result.cost << ","
                 << duration_ms.count() << "\n";

        csv_file.close();
    }

    //std::system("pause");
    return 0;
}