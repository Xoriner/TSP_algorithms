#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <random>
#include <iomanip>
#include <map>
#include <string>

#include "utilities/read_config.h"
#include "utilities/utils.h"
#include "algorithms/tsp_sa.h"

int main(int argc, char* argv[]) {
    std::string config_path = "config.txt";

    if (argc > 1) {
        config_path = argv[1];
    }

    std::ifstream test(config_path);
    if (!test) {
        std::cerr << "Error: Cannot open config file: " << config_path << std::endl;
        return 1;
    }
    test.close();

    auto config = read_config(config_path);

    if (config.find("instancja") == config.end()) {
        std::cerr << "Error: Missing required field 'instancja' in config\n";
        return 1;
    }

    std::vector<std::vector<int>> matrix;
    if (config["instancja"].find(".tsp") != std::string::npos) {
        matrix = read_tsplib(config["instancja"]);
    } else {
        matrix = read_simple_input(config["instancja"]);
    }

    if (matrix.empty()) {
        std::cerr << "Error: Matrix is empty or could not be loaded.\n";
        return 1;
    }

    std::cout << "Loaded instance: " << config["instancja"]
              << " (n = " << matrix.size() << " cities)\n\n";

    SAParams params;
    params.t0 = config.count("t0") ? std::stod(config["t0"]) : 0.0;
    params.lambda = config.count("lambda") ? std::stod(config["lambda"]) : 0.95;
    params.scheme = config.count("schemat") ? config["schemat"] : "geometryczny";
    params.neighborhood = config.count("sasiedztwo") ? config["sasiedztwo"] : "swap";
    params.max_time_s = config.count("max_czas") ? std::stoi(config["max_czas"]) : 900;
    params.max_no_improve = config.count("max_bez_poprawy") ? std::stoi(config["max_bez_poprawy"]) : 50000;
    params.epoch_length = config.count("dlugosc_epoki") ? std::stoi(config["dlugosc_epoki"]) : 100;
    params.use_ub = config.count("wyznacz_UB") && config["wyznacz_UB"] == "true";
    params.use_lb = config.count("wyznacz_LB") && config["wyznacz_LB"] == "true";

    int runs = config.count("powtorzenia") ? std::stoi(config["powtorzenia"]) : 1;

    for(auto const& [key, val] : config) {
        std::cout << "DEBUG: [" << key << "] = [" << val << "]\n";
    }

    std::cout << "======== PARAMETERY ========\n";
    std::cout << "Lambda:        " << std::fixed << std::setprecision(3) << params.lambda << "\n";
    std::cout << "Schemat:       " << params.scheme << "\n";
    std::cout << "Sasiedztwo:    " << params.neighborhood << "\n";
    std::cout << "Dlugosc epoki: " << params.epoch_length << "\n";
    std::cout << "Max czas:      " << params.max_time_s << " s\n";
    std::cout << "Powtorzenia:   " << runs << "\n";
    std::cout << "Wyznacz UB (RNN):  " << (params.use_ub ? "YES" : "NO") << "\n";
    std::cout << "Wyznacz LB (MST):  " << (params.use_lb ? "YES" : "NO") << "\n";
    std::cout << "============================\n\n";

    TSPResult best_overall;
    best_overall.cost = std::numeric_limits<int>::max();

    for (int i = 0; i < runs; i++) {
        std::cout << "Powtorzenie " << (i + 1) << "/" << runs << ":\n";

        TSPResult result = tsp_simulated_annealing(matrix, params);

        std::cout << "  Finalny Koszt: " << result.cost
                  << " | Czas: " << std::fixed << std::setprecision(1)
                  << result.time_ms << " ms";

        if (result.ub != std::numeric_limits<int>::max()) {
            double error = ((double)(result.cost - result.ub) / result.ub) * 100.0;
            std::cout << " | Error: " << std::setprecision(2) << error << "%";
        }
        std::cout << "\n";

        if (result.cost < best_overall.cost) {
            best_overall = result;
        }
    }

    std::cout << "\n======== Najlepszy wynik ========\n";
    std::cout << "Koszt:          " << best_overall.cost << "\n";
    std::cout << "Czas:           " << std::fixed << std::setprecision(1)
              << best_overall.time_ms << " ms\n";

    if (best_overall.ub != std::numeric_limits<int>::max()) {
        std::cout << "UB (RNN):       " << best_overall.ub << "\n";
        double error = ((double)(best_overall.cost - best_overall.ub) / best_overall.ub) * 100.0;
        std::cout << "Roznica od UB:   " << std::fixed << std::setprecision(2) << error << "%\n";
    }

    if (best_overall.lb != std::numeric_limits<int>::max()) {
        std::cout << "LB (MST):       " << best_overall.lb << "\n";
        double gap = ((double)(best_overall.cost - best_overall.lb) / best_overall.lb) * 100.0;
        std::cout << "Roznica od LB:  " << std::fixed << std::setprecision(2) << gap << "%\n";
    }

    std::cout << "============================\n";

    if (config.count("output") && config["output"] == "1") {
        print_solution(best_overall);
    }

    // Zapis do CSV
    std::ofstream csv("results.csv", std::ios::app);
    if (csv.is_open()) {
        csv.seekp(0, std::ios::end);
        if (csv.tellp() == 0) {
            csv << "Instance,Lambda,Scheme,Neighborhood,Cost,UB_RNN,LB_MST,Error%,Gap%,Time_ms\n";
        }

        double error = best_overall.ub != std::numeric_limits<int>::max()
                      ? ((double)(best_overall.cost - best_overall.ub) / best_overall.ub) * 100.0
                      : 0.0;

        double gap = best_overall.lb != std::numeric_limits<int>::max()
                    ? ((double)(best_overall.cost - best_overall.lb) / best_overall.lb) * 100.0
                    : 0.0;

        csv << config["instancja"] << ","
            << std::fixed << std::setprecision(3) << params.lambda << ","
            << params.scheme << ","
            << params.neighborhood << ","
            << best_overall.cost << ","
            << best_overall.ub << ","
            << best_overall.lb << ","
            << std::setprecision(2) << error << ","
            << std::setprecision(2) << gap << ","
            << std::setprecision(1) << best_overall.time_ms << "\n";

        csv.close();
        std::cout << "\nResults saved to results.csv\n";
    }

    return 0;
}