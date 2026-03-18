#include "utils.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>

#include "tsp_result.h"

std::vector<std::vector<int>> read_simple_input(std::string filename) {
    std::ifstream input_file(filename);

    if (!input_file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        exit(1);
    }

    int n;
    if (!(input_file >> n)) {
        std::cerr << "Error reading size from file: " << filename << std::endl;
        exit(1);
    }

    std::vector<std::vector<int>> graph_matrix(n, std::vector<int>(n));

    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            if (!(input_file >> graph_matrix[i][j])) {
                std::cerr << "Error reading matrix at [" << i << "," << j << "]\n";
                exit(1);
            }
        }
    }

    return graph_matrix;
}

std::vector<std::vector<int>> read_tsplib(std::string filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        exit(1);
    }

    std::string line;
    int dimension = 0;

    // Szukamy DIMENSION
    while (std::getline(file, line)) {
        // usuń spacje na początku i końcu
        line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](int ch) {
            return !std::isspace(ch);
        }));
        line.erase(std::find_if(line.rbegin(), line.rend(), [](int ch) {
            return !std::isspace(ch);
        }).base(), line.end());

        // ignoruj wielkość liter
        std::string line_upper = line;
        std::transform(line_upper.begin(), line_upper.end(), line_upper.begin(), ::toupper);

        if (line_upper.find("DIMENSION") != std::string::npos) {
            // znajdź ':' i pobierz liczbę po nim
            auto pos = line.find(":");
            if (pos != std::string::npos) {
                std::string number = line.substr(pos + 1);
                dimension = std::stoi(number);
            } else {
                // np. "DIMENSION 52"
                std::stringstream ss(line);
                std::string tmp;
                ss >> tmp >> dimension;
            }
        }

        if (line_upper.find("NODE_COORD_SECTION") != std::string::npos) {
            break;
        }
    }

    if (dimension == 0) {
        std::cerr << "Error: DIMENSION not found\n";
        exit(1);
    }

    std::vector<std::pair<double, double>> coords(dimension);

    // Wczytaj współrzędne
    for (int i = 0; i < dimension; i++) {
        int id;
        double x, y;

        file >> id >> x >> y;
        coords[id - 1] = {x, y}; // TSPLIB jest 1-indexed
    }

    // Budujemy macierz odległości
    std::vector<std::vector<int>> matrix(dimension, std::vector<int>(dimension));

    for (int i = 0; i < dimension; i++) {
        for (int j = 0; j < dimension; j++) {
            double dx = coords[i].first - coords[j].first;
            double dy = coords[i].second - coords[j].second;

            matrix[i][j] = (int) std::round(std::sqrt(dx * dx + dy * dy));
        }
    }

    return matrix;
}

void print_solution(const TSPResult& result) {
    std::cout << "Cost: " << result.cost << "\n";

    std::cout << "Path: ";
    for (size_t i = 0; i < result.path.size(); i++) {
        std::cout << result.path[i];
        if (i != result.path.size() - 1)
            std::cout << " -> ";
    }
    std::cout << "\n";
}
