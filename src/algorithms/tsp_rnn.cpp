#include "tsp_rnn.h"
#include "../utilities/tsp_nn_equal.h"
#include <limits>
#include <iostream>
#include <iomanip>

TSPResult tsp_rnn(const std::vector<std::vector<int>>& matrix) {
    int n = matrix.size();
    TSPResult global_best;
    global_best.cost = std::numeric_limits<int>::max();

    // Uruchom NN_equal z każdego miasta startowego
    for (int start = 0; start < n; start++) {
        std::vector<bool> visited(n, false);
        std::vector<int> path;

        visited[start] = true;
        path.push_back(start);

        TSPResult local_result;
        local_result.cost = std::numeric_limits<int>::max();

        // Rekursywne szukanie z równymi sąsiadami
        tsp_nn_equal(matrix, visited, path, start, 0, start, local_result);

        if (local_result.cost < global_best.cost) {
            global_best = local_result;
        }
    }

    return global_best;
}