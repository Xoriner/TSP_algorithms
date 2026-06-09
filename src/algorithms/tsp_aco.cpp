#include "algorithms/tsp_aco.h"
#include <cmath>
#include <random>
#include <chrono>
#include <algorithm>
#include <iostream>
#include "tsp_nn.h"

TSPResult tsp_aco(const std::vector<std::vector<int>>& matrix, const ACOParams& params) {
    auto start_time = std::chrono::high_resolution_clock::now();
    int n = matrix.size();
    TSPResult result;

    if (n == 0) return result;

    /* INICJALIZACJA */
    double tau_0;
    if (params.tau0_mode == "nn") {
        int nn_cost = tsp_nearest_neighbor(matrix).cost;
        tau_0 = (double)params.m / (nn_cost > 0 ? nn_cost : 1.0);
    } else {
        tau_0 = params.tau0;
    }

    std::vector<std::vector<double>> tau(n, std::vector<double>(n, tau_0));
    std::vector<std::vector<double>> eta(n, std::vector<double>(n, 0.0));

    // Widoczność krawędzi: eta = 1 / d
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i != j && matrix[i][j] > 0) {
                eta[i][j] = 1.0 / matrix[i][j];
            } else {
                eta[i][j] = 0.0;
            }
        }
    }

    std::vector<std::vector<double>> choice_info(n, std::vector<double>(n, 0.0));

    // Funkcja pre-kalkulacyjna dla wariantu CAS
    auto update_choice_table_CAS = [&]() {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i != j && matrix[i][j] > 0) {
                    choice_info[i][j] = std::pow(tau[i][j], params.alfa) * std::pow(eta[i][j], params.beta);
                } else {
                    choice_info[i][j] = 0.0;
                }
            }
        }
    };

    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dist_city(0, n - 1);
    std::uniform_real_distribution<double> dist_prob(0.0, 1.0);

    int best_cost = std::numeric_limits<int>::max();
    std::vector<int> best_path;
    int no_improve_counter = 0;
    int cycle = 0;

    /* GŁÓWNA PĘTLA */
    while (true) {
        auto current_time = std::chrono::high_resolution_clock::now();
        double elapsed_s = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();
        double elapsed_ms = std::chrono::duration<double, std::milli>(current_time - start_time).count();

        if (elapsed_s >= params.max_time_s) break;
        if (no_improve_counter >= params.max_no_improve) break;

        /* Optymalizacja wag (TYLKO wariant CAS) */
        if (params.wariant == "CAS") {
            update_choice_table_CAS();
        }

        std::vector<std::vector<int>> ant_tours(params.m, std::vector<int>(n));
        std::vector<int> ant_costs(params.m, 0);

        /* Budowanie tras przez mrówki */
        for (int k = 0; k < params.m; ++k) {
            std::vector<bool> visited(n, false);

            int start_node = dist_city(gen);
            ant_tours[k][0] = start_node;
            visited[start_node] = true;

            int current_node = start_node;

            for (int step = 1; step < n; ++step) {
                double sum_prob = 0.0;
                std::vector<double> probs(n, 0.0);

                // Wyznaczanie mianownika i liczników
                for (int next_node = 0; next_node < n; ++next_node) {
                    if (!visited[next_node]) {
                        if (params.wariant == "CAS") {
                            // Użycie pre-kalkulowanych wag
                            probs[next_node] = choice_info[current_node][next_node];
                        } else {
                            // DAS / QAS: Obliczanie na bieżąco uwzględniając najnowszy ślad
                            probs[next_node] = std::pow(tau[current_node][next_node], params.alfa) * std::pow(eta[current_node][next_node], params.beta);
                        }
                        sum_prob += probs[next_node];
                    }
                }

                int selected_node = -1;

                // Ruletka
                if (sum_prob > 0.0) {
                    double r = dist_prob(gen) * sum_prob;
                    double current_sum = 0.0;
                    for (int next_node = 0; next_node < n; ++next_node) {
                        if (!visited[next_node]) {
                            current_sum += probs[next_node];
                            if (current_sum >= r) {
                                selected_node = next_node;
                                break;
                            }
                        }
                    }
                }

                // Fallback w przypadku błędów numerycznych
                if (selected_node == -1) {
                    for (int next_node = 0; next_node < n; ++next_node) {
                        if (!visited[next_node]) {
                            selected_node = next_node;
                            break;
                        }
                    }
                }

                /* Aktualizacja lokalna dla DAS / QAS */
                if (params.wariant == "DAS") {
                    tau[current_node][selected_node] = (1.0 - params.rho) * tau[current_node][selected_node] + params.Q;
                    // Dla symetrii, zaktualizuj też odbicie
                    tau[selected_node][current_node] = tau[current_node][selected_node];
                } else if (params.wariant == "QAS") {
                    double delta = params.Q / matrix[current_node][selected_node];
                    tau[current_node][selected_node] = (1.0 - params.rho) * tau[current_node][selected_node] + delta;
                    tau[selected_node][current_node] = tau[current_node][selected_node];
                }

                ant_tours[k][step] = selected_node;
                ant_costs[k] += matrix[current_node][selected_node];
                visited[selected_node] = true;
                current_node = selected_node;
            }

            // Zamknięcie cyklu
            ant_costs[k] += matrix[current_node][start_node];

            /* Aktualizacja rekordu */
            if (ant_costs[k] < best_cost) {
                best_cost = ant_costs[k];
                best_path = ant_tours[k];
                no_improve_counter = 0;

                if (params.zapis_historii) {
                    result.history.push_back({elapsed_ms, best_cost});
                }
            }
        }

        /* Aktualizacja globalna po cyklu (TYLKO CAS) */
        if (params.wariant == "CAS") {
            // A. Wyparowanie feromonu
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    tau[i][j] *= (1.0 - params.rho);
                    if (tau[i][j] < 1e-8) tau[i][j] = 1e-8; // limit unikania zer
                }
            }

            // B. Naniesienie nowego feromonu na podstawie tras mrówek
            for (int k = 0; k < params.m; ++k) {
                double delta_tau = params.Q / (ant_costs[k] > 0 ? ant_costs[k] : 1.0);
                for (int step = 0; step < n; ++step) {
                    int from = ant_tours[k][step];
                    int to = ant_tours[k][(step + 1) % n];
                    tau[from][to] += delta_tau;
                    tau[to][from] += delta_tau; // Krawędzie symetryczne
                }
            }
        }

        no_improve_counter++;
        cycle++;

        // Zabezpieczenie limitu optymalnego z konfiguracji (Jeśli znaleziono OPT)
        if (params.opt > 0 && best_cost <= params.opt) break;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    result.cost = best_cost;
    result.path = best_path;
    result.time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    // Dodanie końcowego pomiaru do historii
    if (params.zapis_historii) {
        result.history.push_back({result.time_ms, best_cost});
    }

    return result;
}