#include "utils.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <iomanip>

std::vector<std::vector<int>> read_tsplib(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Cannot open TSPLIB file: " << filename << std::endl;
        exit(1);
    }

    std::string line;
    int dimension = 0;

    // Szukaj DIMENSION
    while (std::getline(file, line)) {
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        std::string line_upper = line;
        std::transform(line_upper.begin(), line_upper.end(),
                      line_upper.begin(), ::toupper);

        if (line_upper.find("DIMENSION") != std::string::npos) {
            size_t colon_pos = line.find(':');
            if (colon_pos != std::string::npos) {
                std::string num_str = line.substr(colon_pos + 1);
                dimension = std::stoi(num_str);
            }
        }

        if (line_upper.find("NODE_COORD_SECTION") != std::string::npos) {
            break;
        }
    }

    if (dimension == 0) {
        std::cerr << "Error: DIMENSION not found in TSPLIB file\n";
        exit(1);
    }

    std::vector<std::pair<double, double>> coords(dimension);

    // Wczytaj współrzędne
    for (int i = 0; i < dimension; i++) {
        int id;
        double x, y;
        if (!(file >> id >> x >> y)) {
            std::cerr << "Error: Could not read coordinate " << i << "\n";
            exit(1);
        }
        coords[id - 1] = {x, y};
    }

    // Buduj macierz odległości euklidesowych
    std::vector<std::vector<int>> matrix(dimension,
                                         std::vector<int>(dimension, 0));

    for (int i = 0; i < dimension; i++) {
        for (int j = 0; j < dimension; j++) {
            if (i == j) {
                matrix[i][j] = 0;
            } else {
                double dx = coords[i].first - coords[j].first;
                double dy = coords[i].second - coords[j].second;
                int dist = (int)std::round(std::sqrt(dx * dx + dy * dy));
                matrix[i][j] = dist;
            }
        }
    }

    return matrix;
}

std::vector<std::vector<int>> read_simple_input(const std::string& filename) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file: " << filename << std::endl;
        exit(1);
    }

    int n;
    if (!(file >> n)) {
        std::cerr << "Error: Could not read dimension\n";
        exit(1);
    }

    std::vector<std::vector<int>> matrix(n, std::vector<int>(n));

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (!(file >> matrix[i][j])) {
                std::cerr << "Error: Could not read matrix element ["
                         << i << "," << j << "]\n";
                exit(1);
            }
        }
    }

    file.close();
    return matrix;
}

void print_solution(const TSPResult& result) {
    std::cout << "\nPath (first 20 cities): ";
    for (size_t i = 0; i < std::min((size_t)20, result.path.size()); i++) {
        std::cout << result.path[i];
        if (i < std::min((size_t)19, result.path.size() - 1)) {
            std::cout << " -> ";
        }
    }
    if (result.path.size() > 20) {
        std::cout << " ... -> " << result.path.back();
    }
    std::cout << "\nCost: " << result.cost << "\n";
    std::cout << "Time: " << std::fixed << std::setprecision(2)
              << result.time_ms << " ms\n";
}