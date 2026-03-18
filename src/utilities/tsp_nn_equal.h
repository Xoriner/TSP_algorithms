#ifndef TSP_NN_EQUAL_H
#define TSP_NN_EQUAL_H

#include "tsp_result.h"

void tsp_nn_equal(
    const std::vector<std::vector<int>>& matrix,
    std::vector<bool>& visited,
    std::vector<int>& current_path,
    int current_node,
    int current_cost,
    int start_node,
    TSPResult& best_result
);

#endif //TSP_NN_EQUAL_H
