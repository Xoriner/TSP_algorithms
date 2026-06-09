#pragma once

#include <vector>
#include <limits>

// Struktura przechowująca wyniki algorytmu
struct TSPResult {
    int cost = std::numeric_limits<int>::max();
    std::vector<int> path;
    double time_ms = 0.0;
    int ub = std::numeric_limits<int>::max();
    int lb = std::numeric_limits<int>::max();

    // Historia poprawy: para <czas_w_ms, koszt>
    std::vector<std::pair<double, int>> history;
};