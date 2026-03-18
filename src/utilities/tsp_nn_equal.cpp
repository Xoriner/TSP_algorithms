#pragma once
#include <limits>

#include "tsp_nn_equal.h"

void tsp_nn_equal(
    const std::vector<std::vector<int>>& matrix,
    std::vector<bool>& visited,
    std::vector<int>& current_path,
    int current_node,
    int current_cost,
    int start_node,
    TSPResult& best_result
) {
    int n = matrix.size();

    // jeśli odwiedziliśmy wszystkie miasta
    if (current_path.size() == n) {
        int total_cost = current_cost + matrix[current_node][start_node];

        if (total_cost < best_result.cost) {
            best_result.cost = total_cost;
            best_result.path = current_path;
            best_result.path.push_back(start_node);
        }
        return;
    }

    int min_dist = std::numeric_limits<int>::max();

    // znajdź minimalny dystans
    for (int i = 0; i < n; i++) {
        if (!visited[i] && matrix[current_node][i] < min_dist) {
            min_dist = matrix[current_node][i];
        }
    }

    // zbierz wszystkich kandydatów z minimalnym dystansem
    for (int i = 0; i < n; i++) {
        if (!visited[i] && matrix[current_node][i] == min_dist) {

            visited[i] = true;
            current_path.push_back(i);

            tsp_nn_equal(
                matrix,
                visited,
                current_path,
                i,
                current_cost + matrix[current_node][i],
                start_node,
                best_result
            );

            // backtracking
            visited[i] = false;
            current_path.pop_back();
        }
    }
}