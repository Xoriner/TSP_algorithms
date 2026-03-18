#include <random>
#include "tsp_rand.h"

#include <algorithm>

TSPResult tsp_rand(const std::vector<std::vector<int>>& matrix) {
    int n = matrix.size();
    std::vector<int> nodes;

    for (int i = 1; i < n; i++)
        nodes.push_back(i);

    std::random_device rd;
    std::mt19937 gen(rd());

    int best_cost = std::numeric_limits<int>::max();
    std::vector<int> best_path;


    std::shuffle(nodes.begin(), nodes.end(), gen);

    int cost = 0;
    int prev = 0;

    std::vector<int> path = {0};

    for (int v : nodes) {
        cost += matrix[prev][v];
        prev = v;
        path.push_back(v);
    }

    cost += matrix[prev][0];
    path.push_back(0);

    if (cost < best_cost) {
        best_cost = cost;
        best_path = path;
    }

    return {best_cost, best_path};
}
