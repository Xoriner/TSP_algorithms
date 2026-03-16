#ifndef TSP_RESULT_H
#define TSP_RESULT_H

#include <vector>

struct TSPResult {
    int cost;
    std::vector<int> path;

    //constructor
    TSPResult(int c = 0, std::vector<int> p = {})
        : cost(c), path(std::move(p)) {}
};

#endif