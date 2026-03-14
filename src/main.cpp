#include <iostream>
#include <vector>

#include "tsp_bruteforce.h"

int main() {
    std::vector<std::vector<int>> dist = {
        {0,10,15,20},
        {10,0,35,25},
        {15,35,0,30},
        {20,25,30,0}
    };

    std::cout << "Best cost: " << tsp_bruteforce(dist) << std::endl;
    return 0;
}