#pragma once

#include <vector>
#include <limits>

struct TSPResult {
    int cost = 0;
    std::vector<int> path;
    double time_ms = 0.0;
    int ub = std::numeric_limits<int>::max();  // Upper Bound (RNN)
    int lb = std::numeric_limits<int>::max();  // Lower Bound (MST)
};

#include <limits>