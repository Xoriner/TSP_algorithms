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
// ZMIENIONE: Podłącz swój plik nagłówkowy dla ACO
#include "algorithms/tsp_aco.h"

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
    std::string instancja = config["instancja"];

    // Sprawdzamy czy plik to format TSPLIB (.tsp lub .atsp)
    if (instancja.find(".tsp") != std::string::npos || instancja.find(".atsp") != std::string::npos) {
        matrix = read_tsplib(instancja);
    } else {
        matrix = read_simple_input(instancja);
    }

    if (matrix.empty()) {
        std::cerr << "Error: Matrix is empty or could not be loaded.\n";
        return 1;
    }

    std::cout << "Loaded instance: " << config["instancja"]
              << " (n = " << matrix.size() << " cities)\n\n";

    // ZMIENIONE: Struktura parametrów dla ACO
    ACOParams params;
    params.alfa = config.count("alfa") ? std::stod(config["alfa"]) : 1.0;
    params.beta = config.count("beta") ? std::stod(config["beta"]) : 3.0;
    params.rho = config.count("rho") ? std::stod(config["rho"]) : 0.5;
    // Jeśli brak parametru m, domyślnie bierzemy liczbę miast (częsta praktyka) lub 50
    params.m = config.count("m") ? std::stoi(config["m"]) : std::min(50, (int)matrix.size());

    params.max_time_s = config.count("max_czas") ? std::stoi(config["max_czas"]) : 900;
    params.max_no_improve = config.count("max_bez_poprawy") ? std::stoi(config["max_bez_poprawy"]) : 5000;
    params.target_error = config.count("docelowy_blad") ? std::stod(config["docelowy_blad"]) : 0.0;

    params.use_ub = config.count("wyznacz_UB") && config["wyznacz_UB"] == "true";
    params.use_lb = config.count("wyznacz_LB") && config["wyznacz_LB"] == "true";

    int runs = config.count("powtorzenia") ? std::stoi(config["powtorzenia"]) : 1;

    // ZMIENIONE: Wyświetlanie parametrów mrówkowych
    std::cout << "======== PARAMETERY ACO ========\n";
    std::cout << "Alfa (feromon):    " << std::fixed << std::setprecision(2) << params.alfa << "\n";
    std::cout << "Beta (heurystyka): " << params.beta << "\n";
    std::cout << "Rho (parowanie):   " << params.rho << "\n";
    std::cout << "Liczba mrowek (m): " << params.m << "\n";
    std::cout << "Max czas:          " << params.max_time_s << " s\n";
    std::cout << "Docelowy blad:     " << (params.target_error * 100.0) << "%\n";
    std::cout << "Powtorzenia:       " << runs << "\n";
    std::cout << "Wyznacz UB (RNN):  " << (params.use_ub ? "YES" : "NO") << "\n";
    std::cout << "Wyznacz LB (MST):  " << (params.use_lb ? "YES" : "NO") << "\n";
    std::cout << "================================\n\n";

    TSPResult best_overall;
    best_overall.cost = std::numeric_limits<int>::max();

    for (int i = 0; i < runs; i++) {
        std::cout << "Powtorzenie " << (i + 1) << "/" << runs << ":\n";

        TSPResult result = tsp_aco(matrix, params);

        std::cout << "  Finalny Koszt: " << result.cost
                  << " | Czas: " << std::fixed << std::setprecision(1)
                  << result.time_ms << " ms";

        if (result.ub != std::numeric_limits<int>::max() && result.ub > 0) {
            double error = ((double)(result.cost - result.ub) / result.ub) * 100.0;
            std::cout << " | Error do UB: " << std::setprecision(2) << error << "%";
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

    if (best_overall.ub != std::numeric_limits<int>::max() && best_overall.ub > 0) {
        std::cout << "UB (RNN):       " << best_overall.ub << "\n";
        double error = ((double)(best_overall.cost - best_overall.ub) / best_overall.ub) * 100.0;
        std::cout << "Roznica od UB:  " << std::fixed << std::setprecision(2) << error << "%\n";
    }

    if (best_overall.lb != std::numeric_limits<int>::max() && best_overall.lb > 0) {
        std::cout << "LB (MST):       " << best_overall.lb << "\n";
        double gap = ((double)(best_overall.cost - best_overall.lb) / best_overall.lb) * 100.0;
        std::cout << "Roznica od LB:  " << std::fixed << std::setprecision(2) << gap << "%\n";
    }

    std::cout << "=================================\n";

    if (config.count("output") && config["output"] == "1") {
        print_solution(best_overall);
    }

    std::ofstream csv("results.csv", std::ios::app);
    if (csv.is_open()) {
        csv.seekp(0, std::ios::end);
        if (csv.tellp() == 0) {
            csv << "Instance,Alfa,Beta,Rho,Ants,Cost,UB_RNN,LB_MST,Error%,Gap%,Time_ms\n";
        }

        double error = (best_overall.ub != std::numeric_limits<int>::max() && best_overall.ub > 0)
                      ? ((double)(best_overall.cost - best_overall.ub) / best_overall.ub) * 100.0
                      : 0.0;

        double gap = (best_overall.lb != std::numeric_limits<int>::max() && best_overall.lb > 0)
                    ? ((double)(best_overall.cost - best_overall.lb) / best_overall.lb) * 100.0
                    : 0.0;

        csv << config["instancja"] << ","
            << std::fixed << std::setprecision(2) << params.alfa << ","
            << params.beta << ","
            << params.rho << ","
            << params.m << ","
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