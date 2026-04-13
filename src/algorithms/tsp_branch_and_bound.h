#ifndef TSP_BRANCH_AND_BOUND_H
#define TSP_BRANCH_AND_BOUND_H
#include <vector>
#include "../utilities/tsp_result.h"

TSPResult tsp_branch_and_bound(const std::vector<std::vector<int>>& matrix, int search_mode, int initial_ub);
#endif //TSP_BRANCH_AND_BOUND_H
