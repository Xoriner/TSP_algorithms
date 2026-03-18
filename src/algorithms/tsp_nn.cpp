#include <vector>
#include <limits>
#include <algorithm>

#include "tsp_nn.h"
#include "../utilities/tsp_nn_equal.h"

TSPResult tsp_nearest_neighbor(const std::vector<std::vector<int>>& matrix, int start_node) {
    int n = matrix.size();
    std::vector<bool> visited(n, false);
    std::vector<int> best_path;
    int best_cost = 0;
    int current_node = start_node;

    visited[start_node] = true;
    best_path.push_back(start_node);

    for (int step = 1; step < n; ++step) {
        int next_node = -1;
        int min_dist = std::numeric_limits<int>::max();

        for (int i = 0; i < n; ++i) {
            if (!visited[i] && matrix[current_node][i] < min_dist) {
                min_dist = matrix[current_node][i];
                next_node = i;
            }
        }

        if (next_node != -1) {
            visited[next_node] = true;
            best_path.push_back(next_node);
            best_cost += min_dist;
            current_node = next_node;
        }
    }

    // Powrót do startu
    best_cost += matrix[current_node][start_node];
    best_path.push_back(start_node);

    return {best_cost, best_path};
}