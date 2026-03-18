#include "../algorithms/tsp_rnn.h"
#include "../utilities/tsp_nn_equal.h"
#include "limits"

TSPResult tsp_rnn(const std::vector<std::vector<int>>& matrix) {
    int n = matrix.size();
    TSPResult global_best;
    global_best.cost = std::numeric_limits<int>::max();

    for (int start = 0; start < n; start++) {
        std::vector<bool> visited(n, false);
        std::vector<int> path;

        visited[start] = true;
        path.push_back(start);

        TSPResult local_best_for_start;
        local_best_for_start.cost = std::numeric_limits<int>::max();

        // Wywołujemy wersję z backtrackingiem dla każdego punktu startowego
        tsp_nn_equal(matrix, visited, path, start, 0, start, local_best_for_start);

        if (local_best_for_start.cost < global_best.cost) {
            global_best = local_best_for_start;
        }
    }

    return global_best;
}
