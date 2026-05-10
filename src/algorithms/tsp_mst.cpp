#include "tsp_mst.h"
#include <limits>
#include <algorithm>

int compute_mst_lower_bound(const std::vector<std::vector<int>>& matrix) {
    int n = matrix.size();
    std::vector<bool> in_mst(n, false);
    std::vector<int> min_edge(n, std::numeric_limits<int>::max());

    min_edge[0] = 0;
    int mst_weight = 0;

    // Algorytm Prima
    for (int i = 0; i < n; i++) {
        int u = -1;
        for (int j = 0; j < n; j++) {
            if (!in_mst[j] && (u == -1 || min_edge[j] < min_edge[u])) {
                u = j;
            }
        }

        in_mst[u] = true;
        mst_weight += min_edge[u];

        for (int v = 0; v < n; v++) {
            if (!in_mst[v] && matrix[u][v] < min_edge[v]) {
                min_edge[v] = matrix[u][v];
            }
        }
    }

    return mst_weight;
}