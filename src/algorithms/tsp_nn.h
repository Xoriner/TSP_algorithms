#pragma once

#include "tsp_result.h"
#include <vector>

TSPResult tsp_nearest_neighbor(const std::vector<std::vector<int>>& matrix,
                                int start_city = 0);