#pragma once

#include "tsp_result.h"
#include <vector>
#include <string>

/**
 * Parametry algorytmu Simulated Annealing
 */
struct SAParams {
    double t0;                  // temperatura początkowa
    double lambda;              // współczynnik chłodzenia (0.85-0.999)
    std::string scheme;         // "geometric" lub "logarithmic"
    std::string neighborhood;   // "swap", "insert", "inverse"
    int max_time_s;             // limit czasowy [s]
    int max_no_improve;         // max iteracji bez poprawy
    int epoch_length;           // długość epoki
    bool use_ub;                // wyznaczać UB (RNN)
    bool use_lb;                // wyznaczać LB (MST)
};

/**
 * Simulated Annealing dla TSP z RNN (UB) i MST (LB)
 */
TSPResult tsp_simulated_annealing(const std::vector<std::vector<int>>& matrix,
                                   const SAParams& params);