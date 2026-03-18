#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <random>
#include <limits>
#include <string>
#include <iomanip>

#include "utilities/read_config.h"
#include "utilities/utils.h"
#include "algorithms/tsp_bruteforce.h"
#include "algorithms/tsp_rand.h"
#include "algorithms/tsp_nn.h"
#include "algorithms/tsp_rnn.h"

int main(int argc, char* argv[]) {

    std::string config_path = "config.txt";

    // Jeśli podano argument, używamy go jako ścieżki do konfiguracji
    if (argc > 1) {
        config_path = argv[1];
    }

    // Sprawdzenie czy plik istnieje
    std::ifstream test(config_path);
    if (!test) {
        std::cerr << "Error: Cannot open config file: " << config_path << std::endl;
        return 1;
    }

    auto config = read_config(config_path);

    // Sprawdzenie wymaganych pól
    if (config.find("input_file") == config.end() ||
        config.find("algorithm") == config.end()) {
        std::cerr << "Error: Missing required config fields (input_file / algorithm)\n";
        return 1;
    }

    std::vector<std::vector<int>> matrix;

    // Wczytywanie macierzy kosztów
    if (config["input_file"].find(".tsp") != std::string::npos) {
        matrix = read_tsplib(config["input_file"]);
    } else {
        matrix = read_simple_input(config["input_file"]);
    }

    if (matrix.empty()) {
        std::cerr << "Error: Matrix is empty or could not be loaded.\n";
        return 1;
    }

    std::string algo = config["algorithm"];
    int runs = config.count("runs") ? std::stoi(config["runs"]) : 1;
    if (runs <= 0) runs = 1;

    // Inicjalizacja generatora losowego dla NN (start_node = -1)
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, (int)matrix.size() - 1);

    TSPResult best_overall_result;
    best_overall_result.cost = std::numeric_limits<int>::max();

    std::cout << "Starting " << algo << " with " << runs << " runs...\n";

    // POMIAR CZASU: Start przed pętlą wykonawczą
    auto start_time = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < runs; ++i) {
        TSPResult current_run;

        if (algo == "BF") {
            current_run = tsp_bruteforce(matrix);
        }
        else if (algo == "NN") {
            int start_node = config.count("start_node") ? std::stoi(config["start_node"]) : 0;

            // Jeśli start_node to -1, losujemy wierzchołek dla każdego przebiegu
            if (start_node == -1) {
                current_run = tsp_nearest_neighbor(matrix, dis(gen));
            } else {
                current_run = tsp_nearest_neighbor(matrix, start_node);
            }
        }
        else if (algo == "RNN") {
            current_run = tsp_rnn(matrix);
        }
        else if (algo == "RAND") {
            current_run = tsp_rand(matrix);
        }
        else {
            std::cerr << "Error: Unknown algorithm: " << algo << std::endl;
            return 1;
        }

        // Aktualizacja najlepszego wyniku (szukamy minimum)
        if (current_run.cost < best_overall_result.cost) {
            best_overall_result = current_run;
        }
    }

    // POMIAR CZASU: Stop po zakończeniu wszystkich powtórzeń
    auto stop_time = std::chrono::high_resolution_clock::now();

    // Obliczenia statystyczne
    std::chrono::duration<double, std::milli> total_duration_ms = stop_time - start_time;
    double avg_time = total_duration_ms.count() / runs;

    // Wyświetlanie wyników
    std::cout << "\n================ RESULT ================\n";
    std::cout << "Algorithm:      " << algo << "\n";
    std::cout << "Runs:           " << runs << "\n";
    std::cout << "Best Cost:      " << best_overall_result.cost << "\n";
    std::cout << "Total Time:     " << total_duration_ms.count() << " ms\n";
    std::cout << "Avg Time/Run:   " << avg_time << " ms\n";
    std::cout << "========================================\n";

    if (config.count("output") && config["output"] == "1") {
        std::cout << "Best Path: ";
        print_solution(best_overall_result);
    }

    // ======== Zapis do CSV ========
    std::ofstream csv_file("results.csv", std::ios::app); // dopisywanie
    if (csv_file.is_open()) {
        // jeśli plik był pusty, dodaj nagłówki
        csv_file.seekp(0, std::ios::end);
        if (csv_file.tellp() == 0) {
            csv_file << "Algorithm,Runs,BestCost,TotalTime_ms,AvgTime_ms\n";
        }

        csv_file << algo << ",";
        csv_file << runs << ",";
        csv_file << best_overall_result.cost << ",";
        csv_file << std::fixed << std::setprecision(2) << total_duration_ms.count() << ",";
        csv_file << std::fixed << std::setprecision(2) << avg_time << "\n";

        csv_file.close();
        std::cout << "Results saved to results.csv\n";
    } else {
        std::cerr << "Error: Could not open results.csv for writing\n";
    }

    //Czekanie na klikniecie
    std::system("pause");


    return 0;
}