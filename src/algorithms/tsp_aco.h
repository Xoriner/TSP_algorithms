#ifndef TSP_ACO_H
#define TSP_ACO_H

#include <string>
#include <vector>


#include "tsp_result.h"
// Struktura przechowująca parametry konfiguracyjne algorytmu ACO
struct ACOParams {
    double alfa;
    double beta;
    double rho;
    double Q;
    int m;

    std::string wariant;     // "CAS", "DAS", "QAS"
    std::string tau0_mode;   // "nn", "manual"
    double tau0;             // Używane gdy tau0_mode == "manual"

    int max_time_s;
    int max_no_improve;
    bool use_ub;
    bool use_lb;
    bool zapis_historii;
    int opt;                 // Oczekiwana wartość optymalna (OPT)
};

TSPResult tsp_aco(const std::vector<std::vector<int>>& matrix, const ACOParams& params);

#endif //TSP_ACO_H