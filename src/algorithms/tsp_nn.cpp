#include "tsp_nn.h"
#include <algorithm>
#include <limits>

TSPResult tsp_nearest_neighbor(const std::vector<std::vector<int>>& matrix,
                                int start_city) {
    int n = matrix.size();
    TSPResult result;
    result.path.push_back(start_city);

    std::vector<bool> visited(n, false);
    visited[start_city] = true;
    int current = start_city;
    int total_cost = 0;

    for (int i = 1; i < n; i++) {
        int nearest = -1;
        int min_dist = std::numeric_limits<int>::max();

        for (int j = 0; j < n; j++) {
            if (!visited[j] && matrix[current][j] < min_dist) {
                min_dist = matrix[current][j];
                nearest = j;
            }
        }

        visited[nearest] = true;
        total_cost += min_dist;
        result.path.push_back(nearest);
        current = nearest;
    }

    // Powrót do miasta startowego
    total_cost += matrix[current][start_city];
    result.cost = total_cost;

    return result;
}