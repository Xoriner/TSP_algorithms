#include <vector>
#include <limits>
#include <algorithm>

#include "tsp_nn.h"

TSPResult tsp_nearest_neighbor(const std::vector<std::vector<int>>& matrix, int start_node) {
    int n = matrix.size();
    std::vector<bool> visited(n, false);
    std::vector<int> path;
    int best_cost = 0;

    int current_node = start_node;
    path.push_back(current_node);
    visited[current_node] = true;

    for (int i = 0; i < n - 1; ++i) {
        int next_node = -1;
        int min_distance = std::numeric_limits<int>::max();

        // Szukamy najbliższego nieodwiedzonego sąsiada
        for (int neighbor = 0; neighbor < n; ++neighbor) {
            if (!visited[neighbor] && matrix[current_node][neighbor] < min_distance) {
                min_distance = matrix[current_node][neighbor];
                next_node = neighbor;
            }
        }

        if (next_node != -1) {
            path.push_back(next_node);
            visited[next_node] = true;
            best_cost += min_distance;
            current_node = next_node;
        }
    }

    // Powrót do miasta startowego
    best_cost += matrix[current_node][start_node];
    path.push_back(start_node);

    return {best_cost, path};
}