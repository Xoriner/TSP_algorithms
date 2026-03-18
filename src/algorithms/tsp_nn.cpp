#include <vector>
#include <limits>
#include <algorithm>

#include "tsp_nn.h"
#include "../utilities/tsp_nn_equal.h"

TSPResult tsp_nearest_neighbor(const std::vector<std::vector<int>>& matrix, int start_node) {
    int n = matrix.size();

    std::vector<bool> visited(n, false);
    std::vector<int> path;

    visited[start_node] = true;
    path.push_back(start_node);

    TSPResult best_result;
    best_result.cost = std::numeric_limits<int>::max();

    tsp_nn_equal(
        matrix,
        visited,
        path,
        start_node,
        0,
        start_node,
        best_result
    );

    return best_result;
}