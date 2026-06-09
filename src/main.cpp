#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>
#include <random>
#include <iomanip>
#include <map>
#include <string>
#include <algorithm>

#include "utilities/read_config.h"
#include "utilities/utils.h"
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

    // Wczytywanie instancji
    if (instancja.find(".tsp") != std::string::npos || instancja.find(".atsp") != std::string::npos) {
        matrix = read_tsplib(instancja);
    } else {
        matrix = read_simple_input(instancja);
    }

    if (matrix.empty()) {
        std::cerr << "Error: Matrix is empty or could not be loaded.\n";
        return 1;
    }

    int n = matrix.size();
    std::cout << "Loaded instance: " << config["instancja"] << " (n = " << n << " cities)\n\n";

    // Konfiguracja ACO na podstawie sprawozdania
    ACOParams params;
    params.alfa = config.count("alfa") ? std::stod(config["alfa"]) : 1.0;
    params.beta = config.count("beta") ? std::stod(config["beta"]) : 2.0;
    params.rho = config.count("rho") ? std::stod(config["rho"]) : 0.5;
    params.Q = config.count("Q") ? std::stod(config["Q"]) : 1.0;

    // Jeśli m = 0, ustaw na liczbę miast (zgodnie z plikiem konfiguracyjnym)
    int m_in = config.count("m") ? std::stoi(config["m"]) : 0;
    params.m = (m_in == 0) ? n : m_in;

    params.wariant = config.count("wariant") ? config["wariant"] : "CAS";
    params.tau0_mode = config.count("tau0_mode") ? config["tau0_mode"] : "nn";
    params.tau0 = config.count("tau0") ? std::stod(config["tau0"]) : 0.01;

    params.max_time_s = config.count("max_czas") ? std::stoi(config["max_czas"]) : 900;
    params.max_no_improve = config.count("max_bez_poprawy") ? std::stoi(config["max_bez_poprawy"]) : 200;
    params.opt = config.count("OPT") ? std::stoi(config["OPT"]) : -1;
    params.zapis_historii = config.count("zapis_historii") && config["zapis_historii"] == "true";
    params.use_ub = config.count("wyznacz_UB") && config["wyznacz_UB"] == "true";
    params.use_lb = config.count("wyznacz_LB") && config["wyznacz_LB"] == "true";

    int runs = config.count("powtorzenia") ? std::stoi(config["powtorzenia"]) : 1;

    std::cout << "======== PARAMETERY ACO ========\n";
    std::cout << "Wariant:           " << params.wariant << "\n";
    std::cout << "Alfa (feromon):    " << std::fixed << std::setprecision(2) << params.alfa << "\n";
    std::cout << "Beta (heurystyka): " << params.beta << "\n";
    std::cout << "Rho (parowanie):   " << params.rho << "\n";
    std::cout << "Stala Q:           " << params.Q << "\n";
    std::cout << "Liczba mrowek (m): " << params.m << " (n=" << n << ")\n";
    std::cout << "Inicjalizacja tau0:" << params.tau0_mode << (params.tau0_mode == "manual" ? (" (" + std::to_string(params.tau0) + ")") : "") << "\n";
    std::cout << "Max czas:          " << params.max_time_s << " s\n";
    std::cout << "Max bez poprawy:   " << params.max_no_improve << " cykli\n";
    if(params.opt > 0) {
        std::cout << "Cel optymalny (OPT): " << params.opt << "\n";
    }
    std::cout << "Powtorzenia:       " << runs << "\n";
    std::cout << "================================\n\n";

    TSPResult best_overall;
    best_overall.cost = std::numeric_limits<int>::max();

    for (int i = 0; i < runs; i++) {
        std::cout << "Powtorzenie " << (i + 1) << "/" << runs << ":\n";

        TSPResult result = tsp_aco(matrix, params);

        std::cout << "  Finalny Koszt: " << result.cost
                  << " | Czas: " << std::fixed << std::setprecision(1)
                  << result.time_ms << " ms";

        if (params.opt > 0) {
            double delta = ((double)(result.cost - params.opt) / params.opt) * 100.0;
            std::cout << " | Delta (Blad do OPT): " << std::setprecision(2) << delta << "%";
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

    if (params.opt > 0) {
        double delta = ((double)(best_overall.cost - params.opt) / params.opt) * 100.0;
        std::cout << "Blad (Delta):   " << std::fixed << std::setprecision(2) << delta << "%\n";
    }

    std::cout << "=================================\n";

    if (config.count("output") && config["output"] == "1") {
        print_solution(best_overall);
    }

    // Zapis ogólnych wyników do pliku CSV
    std::ofstream csv("results.csv", std::ios::app);
    if (csv.is_open()) {
        csv.seekp(0, std::ios::end);
        if (csv.tellp() == 0) {
            csv << "Instance,Wariant,Alfa,Beta,Rho,Q,Ants,Tau0Mode,Cost,OPT,Delta%,Time_ms\n";
        }

        double delta = (params.opt > 0) ? ((double)(best_overall.cost - params.opt) / params.opt) * 100.0 : 0.0;

        csv << config["instancja"] << ","
            << params.wariant << ","
            << std::fixed << std::setprecision(2) << params.alfa << ","
            << params.beta << ","
            << params.rho << ","
            << params.Q << ","
            << params.m << ","
            << params.tau0_mode << ","
            << best_overall.cost << ","
            << params.opt << ","
            << std::setprecision(2) << delta << ","
            << std::setprecision(1) << best_overall.time_ms << "\n";
        csv.close();
        std::cout << "Results saved to results.csv\n";
    }

    // Zapis historii jeśli ustawiono
    if (params.zapis_historii && !best_overall.history.empty()) {
        std::ofstream hist_csv("history.csv");
        if (hist_csv.is_open()) {
            hist_csv << "Time_ms,BestCost\n";
            for (const auto& point : best_overall.history) {
                hist_csv << std::fixed << std::setprecision(1) << point.first << "," << point.second << "\n";
            }
            hist_csv.close();
            std::cout << "History saved to history.csv\n";
        }
    }

    return 0;
}