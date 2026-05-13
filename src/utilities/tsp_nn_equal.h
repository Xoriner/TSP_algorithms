#pragma once

#include "../algorithms/tsp_result.h"
#include <vector>
#include <limits>

//Nearest Neighbor z rozpatrywaniem równych sąsiadów.

// ZAMIEŃ CAŁĄ FUNKCJĘ NA TĘ WERSJĘ ITERACYJNĄ:
inline void tsp_nn_equal(
    const std::vector<std::vector<int>>& matrix,
    std::vector<bool>& visited,
    std::vector<int>& current_path,
    int current_node,
    int current_cost,
    int start_node,
    TSPResult& best_result
) {
    int n = matrix.size();

    // Klasyczna pętla zachłanna zamiast rekurencji
    for (int step = 1; step < n; step++) {
        int min_dist = std::numeric_limits<int>::max();
        int next_node = -1;

        for (int i = 0; i < n; i++) {
            if (!visited[i] && matrix[current_node][i] < min_dist) {
                min_dist = matrix[current_node][i];
                next_node = i;
            }
        }

        if (next_node == -1) break; // Nie powinno wystąpić w pełnym grafie

        visited[next_node] = true;
        current_path.push_back(next_node);
        current_cost += min_dist;
        current_node = next_node;
    }

    // Zamknięcie cyklu
    int total_cost = current_cost + matrix[current_node][start_node];
    current_path.push_back(start_node);

    // Zapisujemy wynik
    best_result.cost = total_cost;
    best_result.path = current_path;
}