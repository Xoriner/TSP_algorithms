#include <vector>
#include <algorithm>
#include <limits>

#include "tsp_bruteforce.h"

//TODO
//Optymalizacje na zasadzie nie sprawdzania jak wiecej niz best_cost juz sie wyliczylo

TSPResult tsp_bruteforce(const std::vector<std::vector<int>>& graph_matrix) {
    int n = graph_matrix.size();

    std::vector<int> perm;

    // We know we have to come back to the starting point
    // This way we generate (n-1)! permutations not n!
    for (int i = 1; i < n; i++)
        perm.push_back(i);

    int best_cost = std::numeric_limits<int>::max();
    std::vector<int> best_path;

    do {
        int current_cost = 0;
        int prev = 0;

        for (int v : perm) {
            current_cost += graph_matrix[prev][v];
            prev = v;
        }

        current_cost += graph_matrix[prev][0];

        if (current_cost < best_cost) {
            best_cost = current_cost;
            best_path = perm; // kopia najlepszej permutacji
        }

    } while (std::next_permutation(perm.begin(), perm.end()));

    return {best_cost, best_path};
}