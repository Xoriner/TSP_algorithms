#pragma once

#include "tsp_result.h"
#include <vector>

/**
 * Repetitive Nearest Neighbor z równymi sąsiadami (RNN_equal).
 *
 * Uruchamia NN_equal z każdego możliwego miasta startowego
 * i zwraca najlepsze znalezione rozwiązanie.
 *
 * Gwarantuje bardzo dobre górne ograniczenie (UB) dla SA.
 */
TSPResult tsp_rnn(const std::vector<std::vector<int>>& matrix);