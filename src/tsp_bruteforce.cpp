#include <vector>
#include <algorithm>
#include <limits>

#include "tsp_bruteforce.h"


int tsp_bruteforce(const std::vector<std::vector<int>>& dist) {
    int n = dist.size();

    std::vector<int> perm;
    for(int i = 1; i < n; i++)
        perm.push_back(i);

    // for safety max value of int
    int best_cost = std::numeric_limits<int>::max();

    do {
        int cost = 0;
        int prev = 0;

        for(int v : perm) {
            cost += dist[prev][v];
            prev = v;
        }

        cost += dist[prev][0];

        best_cost = std::min(best_cost, cost);

    } while(next_permutation(perm.begin(), perm.end()));

    return best_cost;
}

