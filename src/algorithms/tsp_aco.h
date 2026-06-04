#ifndef TSP_ACO_H
#define TSP_ACO_H

#include <vector>


#include "tsp_result.h"
// Struktura przechowująca parametry algorytmu mrówkowego
struct ACOParams {
    double alfa;
    double beta;
    double rho;
    int m;
    int max_time_s;
    int max_no_improve;
    double target_error;
    bool use_ub;
    bool use_lb;
};

// Główna funkcja uruchamiająca algorytm mrówkowy dla TSP/ATSP
TSPResult tsp_aco(const std::vector<std::vector<int>>& matrix, const ACOParams& params);

#endif //TSP_ACO_H