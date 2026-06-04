#include "tsp_aco.h"
#include <cmath>
#include <random>
#include <chrono>
#include <algorithm>
#include <iostream>

// Funkcja pomocnicza: Heurystyka najbliższego sąsiada (Nearest Neighbor) do wyznaczenia UB i tau_0
int compute_nearest_neighbor(const std::vector<std::vector<int>>& matrix) {
    int n = matrix.size();
    if (n == 0) return 0;

    std::vector<bool> visited(n, false);
    int current = 0;
    visited[current] = true;
    int total_cost = 0;

    for (int step = 1; step < n; ++step) {
        int next_city = -1;
        int min_dist = std::numeric_limits<int>::max();
        for (int i = 0; i < n; ++i) {
            if (!visited[i] && matrix[current][i] < min_dist) {
                min_dist = matrix[current][i];
                next_city = i;
            }
        }
        if (next_city == -1) break;
        total_cost += min_dist;
        current = next_city;
        visited[current] = true;
    }
    total_cost += matrix[current][0]; // powrót do miasta startowego
    return total_cost;
}

// Funkcja pomocnicza: Wyznaczanie dolnej granicy (LB) poprzez sumę minimalnych krawędzi wyjściowych
int compute_lower_bound(const std::vector<std::vector<int>>& matrix) {
    int n = matrix.size();
    int lb = 0;
    for (int i = 0; i < n; ++i) {
        int min_val = std::numeric_limits<int>::max();
        for (int j = 0; j < n; ++j) {
            if (i != j && matrix[i][j] < min_val) {
                min_val = matrix[i][j];
            }
        }
        if (min_val != std::numeric_limits<int>::max()) {
            lb += min_val;
        }
    }
    return lb;
}

TSPResult tsp_aco(const std::vector<std::vector<int>>& matrix, const ACOParams& params) {
    auto start_time = std::chrono::high_resolution_clock::now();
    int n = matrix.size();
    TSPResult result;

    if (n == 0) return result;

    // 1. Wyznaczenie UB oraz LB jeśli są wymagane
    int nn_cost = compute_nearest_neighbor(matrix);
    if (params.use_ub) result.ub = nn_cost;
    if (params.use_lb) result.lb = compute_lower_bound(matrix);

    // 2. Inicjalizacja macierzy feromonów (tau) oraz widoczności (eta)
    // tau_0 = m / L_nn (klasyczna inicjalizacja dla Ant System)
    double tau_0 = (double)params.m / (nn_cost > 0 ? nn_cost : 1.0);
    std::vector<std::vector<double>> tau(n, std::vector<double>(n, tau_0));
    std::vector<std::vector<double>> eta(n, std::vector<double>(n, 0.0));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i != j && matrix[i][j] > 0) {
                eta[i][j] = 1.0 / matrix[i][j];
            } else {
                eta[i][j] = 0.0;
            }
        }
    }

    // Tablica decyzyjna (atraktorów)
    std::vector<std::vector<double>> choice_info(n, std::vector<double>(n, 0.0));

    // Aktualizacja tablicy decyzyjnej przed każdą iteracją (z uwzględnieniem dzielenia)
    auto update_choice_table = [&]() {
        for (int i = 0; i < n; ++i) {
            // 1. Najpierw liczymy mianownik (sumę dla całego wiersza / miasta i)
            double row_sum = 0.0;
            for (int l = 0; l < n; ++l) {
                if (i != l && matrix[i][l] > 0) {
                    row_sum += std::pow(tau[i][l], params.alfa) * std::pow(eta[i][l], params.beta);
                }
            }

            // 2. Teraz wyznaczamy ostateczne wartości a_ij (dzieląc licznik przez mianownik)
            for (int j = 0; j < n; ++j) {
                if (i != j && row_sum > 0.0 && matrix[i][j] > 0) {
                    double numerator = std::pow(tau[i][j], params.alfa) * std::pow(eta[i][j], params.beta);
                    choice_info[i][j] = numerator / row_sum;
                } else {
                    choice_info[i][j] = 0.0;
                }
            }
        }
    };

    // Generator liczb losowych
    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<int> dist_city(0, n - 1);
    std::uniform_real_distribution<double> dist_prob(0.0, 1.0);

    // Zmienne do kontroli kryteriów stopu
    int best_cost = std::numeric_limits<int>::max();
    std::vector<int> best_path;
    int no_improve_counter = 0;

    // Główna pętla algorytmu
    while (true) {
        // Sprawdzenie kryterium czasu
        auto current_time = std::chrono::high_resolution_clock::now();
        double elapsed_s = std::chrono::duration_cast<std::chrono::seconds>(current_time - start_time).count();
        if (elapsed_s >= params.max_time_s) break;

        // Sprawdzenie kryterium braku poprawy
        if (no_improve_counter >= params.max_no_improve) break;

        // KROK 1: Aktualizacja optymalizacyjnej tablicy decyzyjnej na dany cykl
        update_choice_table();

        std::vector<std::vector<int>> ant_tours(params.m, std::vector<int>(n));
        std::vector<int> ant_costs(params.m, 0);

        // KROK 2: Konstrukcja rozwiązań przez mrówki
        for (int k = 0; k < params.m; ++k) {
            std::vector<bool> visited(n, false);

            // Rozmieszczenie mrówek: równomiernie lub losowo jeśli mówek jest więcej niż miast
            int start_node = (params.m <= n) ? (k % n) : dist_city(gen);
            ant_tours[k][0] = start_node;
            visited[start_node] = true;

            int current_node = start_node;

            for (int step = 1; step < n; ++step) {
                // Oblicz sumę prawdopodobieństw dla nieodwiedzonych miast
                double sum_prob = 0.0;
                for (int next_node = 0; next_node < n; ++next_node) {
                    if (!visited[next_node]) {
                        sum_prob += choice_info[current_node][next_node];
                    }
                }

                int selected_node = -1;

                if (sum_prob > 0.0) {
                    // Selekcja ruletkowa przy użyciu stablicowanych wartości
                    double r = dist_prob(gen) * sum_prob;
                    double current_sum = 0.0;
                    for (int next_node = 0; next_node < n; ++next_node) {
                        if (!visited[next_node]) {
                            current_sum += choice_info[current_node][next_node];
                            if (current_sum >= r) {
                                selected_node = next_node;
                                break;
                            }
                        }
                    }
                }

                // Zabezpieczenie przed błędami numerycznymi (underflow) lub izolowanymi węzłami
                if (selected_node == -1) {
                    for (int next_node = 0; next_node < n; ++next_node) {
                        if (!visited[next_node]) {
                            selected_node = next_node;
                            break;
                        }
                    }
                }

                ant_tours[k][step] = selected_node;
                ant_costs[k] += matrix[current_node][selected_node];
                visited[selected_node] = true;
                current_node = selected_node;
            }
            // Koszt powrotu do punktu startowego
            ant_costs[k] += matrix[current_node][start_node];

            // Ocena i zapis najlepszego globalnego rozwiązania
            if (ant_costs[k] < best_cost) {
                best_cost = ant_costs[k];
                best_path = ant_tours[k];
                no_improve_counter = 0; // Reset licznika braku poprawy
            }
        }

        no_improve_counter++;

        // KROK 3: Globalna aktualizacja śladu feromonowego
        // A. Wyparowanie feromonu
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                tau[i][j] *= (1.0 - params.rho);
                // Zabezpieczenie przed całkowitym zanikiem feromonu
                if (tau[i][j] < 1e-6) tau[i][j] = 1e-6;
            }
        }

        // B. Naniesienie nowego feromonu (system CAS / AS)
        for (int k = 0; k < params.m; ++k) {
            double delta_tau = 1.0 / (ant_costs[k] > 0 ? ant_costs[k] : 1.0);
            for (int step = 0; step < n; ++step) {
                int from = ant_tours[k][step];
                int to = ant_tours[k][(step + 1) % n];
                tau[from][to] += delta_tau;
            }
        }

        // Opcjonalne wyjście wcześniejsze w przypadku osiągnięcia błędu docelowego
        // if (params.use_ub && params.target_error > 0.0) {
        //     double current_error = ((double)(best_cost - result.ub) / result.ub);
        //     if (current_error <= params.target_error) break;
        // }
    }

    // Przygotowanie wyników końcowych
    auto end_time = std::chrono::high_resolution_clock::now();
    result.cost = best_cost;
    result.path = best_path;
    result.time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    return result;
}