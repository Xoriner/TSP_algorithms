#include "tsp_sa.h"
#include "tsp_rnn.h"
#include "tsp_mst.h"
#include <chrono>
#include <random>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <limits>

// ========== OPERATORY SĄSIEDZTWA ==========

std::vector<int> neighbor_swap(const std::vector<int>& path,
                               std::mt19937& rng) {
    std::vector<int> new_path = path;
    int n = new_path.size();

    std::uniform_int_distribution<> dis(0, n - 1);
    int i = dis(rng);
    int j = dis(rng);

    std::swap(new_path[i], new_path[j]);
    return new_path;
}

std::vector<int> neighbor_insert(const std::vector<int>& path,
                                 std::mt19937& rng) {
    std::vector<int> new_path = path;
    int n = new_path.size();

    std::uniform_int_distribution<> dis(0, n - 1);
    int i = dis(rng);
    int j = dis(rng);

    if (i != j) {
        int elem = new_path[i];
        new_path.erase(new_path.begin() + i);
        new_path.insert(new_path.begin() + j, elem);
    }
    return new_path;
}

std::vector<int> neighbor_inverse(const std::vector<int>& path,
                                  std::mt19937& rng) {
    std::vector<int> new_path = path;
    int n = new_path.size();

    std::uniform_int_distribution<> dis(0, n - 1);
    int i = dis(rng);
    int j = dis(rng);

    if (i > j) std::swap(i, j);
    std::reverse(new_path.begin() + i, new_path.begin() + j + 1);

    return new_path;
}

// ========== FUNKCJE POMOCNICZE ==========

int calculate_cost(const std::vector<int>& path,
                   const std::vector<std::vector<int>>& matrix) {
    int cost = 0;
    int n = path.size();

    for (int i = 0; i < n; i++) {
        int from = path[i];
        int to = path[(i + 1) % n];
        cost += matrix[from][to];
    }

    return cost;
}

std::vector<int> random_path(int n, std::mt19937& rng) {
    std::vector<int> path(n);
    for (int i = 0; i < n; i++) {
        path[i] = i;
    }
    std::shuffle(path.begin(), path.end(), rng);
    return path;
}

// ========== GŁÓWNY ALGORYTM SA ==========

TSPResult tsp_simulated_annealing(const std::vector<std::vector<int>>& matrix,
                                   const SAParams& params) {
    TSPResult result;
    TSPResult rnn_result;
    result.ub = std::numeric_limits<int>::max();
    result.lb = std::numeric_limits<int>::max();

    // ========== UB: RNN (z równymi sąsiadami) ==========
    if (params.use_ub) {
        rnn_result = tsp_rnn(matrix);
        result.ub = rnn_result.cost;
        std::cout << "  UB (RNN): " << result.ub << "\n";
    }

    // ========== LB: MST ==========
    if (params.use_lb) {
        result.lb = compute_mst_lower_bound(matrix);
        std::cout << "  LB (MST): " << result.lb << "\n";
    }

    int n = matrix.size();
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<> real_dis(0.0, 1.0);

    // ========== INICJALIZACJA TEMPERATURY ==========
    double T0 = params.t0;
    if (T0 == 0) {
        if (params.use_ub) {
            T0 = 0.1 * result.ub;  // TODO powiazanie z 10% UB (RNN)
        } else {
            T0 = 10000.0;
        }
    }

    std::cout << "  T0: " << std::fixed << std::setprecision(1) << T0 << "\n";

    // ========== INICJALIZACJA ROZWIĄZANIA Z RNN LUB RAND ==========
    std::vector<int> current_path;
    int current_cost;

    if (params.use_ub && result.ub != std::numeric_limits<int>::max()) {
        // Zacznij od rozwiązania RNN
        current_path = rnn_result.path;
        current_cost = rnn_result.cost;
        //std::cout << "  UB (RNN): " << current_cost << ")\n";
    } else {
        // Fallback: losowe rozwiązanie
        current_path = random_path(n, rng);
        current_cost = calculate_cost(current_path, matrix);
        //std::cout << "  UB (Random): " << current_cost << ")\n";
    }

    std::vector<int> best_path = current_path;
    int best_cost = current_cost;

    double T = T0;
    int no_improve_count = 0;
    int iteration = 0;

    // ========= START CZASU =======
    auto start_time = std::chrono::high_resolution_clock::now();
    // ========== GŁÓWNA PĘTLA ==========
    while (true) {
        auto current_time = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>
                      (current_time - start_time).count();

        // WARUNKI STOPU
        if (elapsed > params.max_time_s) {
            break;
        }
        if (no_improve_count > params.max_no_improve) {
            break;
        }

        bool found_global_improvement = false;
        // EPOKA
        for (int epoch = 0; epoch < params.epoch_length; epoch++) {
            // Generuj sąsiada
            std::vector<int> neighbor_path;

            if (params.neighborhood == "swap") {
                neighbor_path = neighbor_swap(current_path, rng);
            } else if (params.neighborhood == "insert") {
                neighbor_path = neighbor_insert(current_path, rng);
            } else if (params.neighborhood == "inverse") {
                neighbor_path = neighbor_inverse(current_path, rng);
            } else {
                neighbor_path = neighbor_swap(current_path, rng);
            }

            int neighbor_cost = calculate_cost(neighbor_path, matrix);
            int delta_E = neighbor_cost - current_cost;

            // AKCEPTACJA (kryterium Metropolis)
            bool accept = false;
            if (delta_E < 0) {
                //Rozwiazania lepsze zawsze akceptujemy
                accept = true;
            } else {
                //Rozwiazanie gorsze - zalezy od prawdopodobienstwa
                double prob = std::exp(-((double)delta_E) / T);
                if (real_dis(rng) < prob) {
                    accept = true;
                }
            }

            if (accept) {
                current_path = neighbor_path;
                current_cost = neighbor_cost;

                if (current_cost < best_cost) {
                    best_path = current_path;
                    best_cost = current_cost;
                    found_global_improvement = true; //znaleziono poprawe globalnego
                }
            }
        }

        // Aktualizacja licznika braku popraw po epoce
        if (found_global_improvement) {
            no_improve_count = 0; // Reset, bo znaleźliśmy lepsze rozwiązanie
        } else {
            no_improve_count++; // Zwiększamy, bo ta epoka nie przyniosła poprawy globalnej
        }

        // CHŁODZENIE
        if (params.scheme == "geometryczny") {
            T = T * params.lambda;
        } else if (params.scheme == "logarytmiczny") {
            T = T0 / std::log((double)(iteration + 2));
        } else {
            T = T * params.lambda;
        }

        iteration++;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>
                      (end_time - start_time).count();

    result.path = best_path;
    result.cost = best_cost;
    result.time_ms = duration_ms;

    return result;
}