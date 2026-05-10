#pragma once

#include <vector>

/**
 * Oblicz Lower Bound za pomocą MST (Minimum Spanning Tree).
 * 
 * MST stanowi dolną granicę dla optymalnej trasy TSP,
 * ponieważ TSP jest MST z dodanym jednym krawędzią.
 */
int compute_mst_lower_bound(const std::vector<std::vector<int>>& matrix);