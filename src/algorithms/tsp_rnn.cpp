#include "../algorithms/tsp_rnn.h"
#include "../algorithms/tsp_nn.h"

#include "limits"

TSPResult tsp_rnn(const std::vector<std::vector<int>>& matrix) {
    int n = matrix.size();

    int best_cost = std::numeric_limits<int>::max();
    std::vector<int> best_path;

    for (int start = 0; start < n; start++) {
        TSPResult res = tsp_nearest_neighbor(matrix, start);

        if (res.cost < best_cost) {
            best_cost = res.cost;
            best_path = res.path;
        }
    }

    return {best_cost, best_path};
}