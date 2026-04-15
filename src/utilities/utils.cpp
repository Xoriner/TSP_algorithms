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

// Pomocnicza funkcja do obliczeń geograficznych (standard TSPLIB)
int calc_geo_dist(double x1, double y1, double x2, double y2) {
    double PI = 3.14159265358979323846;
    auto to_rad = [PI](double deg) {
        int d = (int)deg;
        double m = deg - d;
        return PI * (d + 5.0 * m / 3.0) / 180.0;
    };

    double lat1 = to_rad(x1), lon1 = to_rad(y1);
    double lat2 = to_rad(x2), lon2 = to_rad(y2);

    double RRR = 6378.388;
    double q1 = cos(lon1 - lon2);
    double q2 = cos(lat1 - lat2);
    double q3 = cos(lat1 + lat2);
    return (int)(RRR * acos(0.5 * ((1.0 + q1) * q2 - (1.0 - q1) * q3)) + 1.0);
}

std::vector<std::vector<int>> read_tsplib(std::string filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        exit(1);
    }

    std::string line, weight_type = "", weight_format = "";
    int dimension = 0;
    bool is_explicit = false;

    // 1. Parsowanie Nagłówka
    while (std::getline(file, line)) {
        std::string line_upper = line;
        std::transform(line_upper.begin(), line_upper.end(), line_upper.begin(), ::toupper);

        if (line_upper.find("DIMENSION") != std::string::npos) {
            size_t pos = line_upper.find(":");
            dimension = std::stoi(line_upper.substr(pos != std::string::npos ? pos + 1 : 9));
        }
        else if (line_upper.find("EDGE_WEIGHT_TYPE") != std::string::npos) {
            if (line_upper.find("GEO") != std::string::npos) weight_type = "GEO";
            if (line_upper.find("EUC_2D") != std::string::npos) weight_type = "EUC_2D";
            if (line_upper.find("EXPLICIT") != std::string::npos) is_explicit = true;
        }
        else if (line_upper.find("EDGE_WEIGHT_SECTION") != std::string::npos ||
                 line_upper.find("NODE_COORD_SECTION") != std::string::npos) {
            break; // Przechodzimy do czytania danych
        }
    }

    if (dimension == 0) exit(1);
    std::vector<std::vector<int>> matrix(dimension, std::vector<int>(dimension, 0));

    // 2. Wczytywanie danych: Albo Macierz (ATSP / Explicit), Albo Współrzędne (TSP)
    if (is_explicit) {
        // Czytamy macierz bezpośrednio (np. br17)
        for (int i = 0; i < dimension; i++) {
            for (int j = 0; j < dimension; j++) {
                file >> matrix[i][j];
            }
        }
    } else {
        // Czytamy współrzędne (np. ulysses, berlin52)
        std::vector<std::pair<double, double>> coords(dimension);
        for (int i = 0; i < dimension; i++) {
            int id;
            file >> id >> coords[i].first >> coords[i].second;
        }

        for (int i = 0; i < dimension; i++) {
            for (int j = 0; j < dimension; j++) {
                if (i == j) {
                    matrix[i][j] = 0;
                } else if (weight_type == "GEO") {
                    matrix[i][j] = calc_geo_dist(coords[i].first, coords[i].second, coords[j].first, coords[j].second);
                } else { // EUC_2D domyślnie
                    double dx = coords[i].first - coords[j].first;
                    double dy = coords[i].second - coords[j].second;
                    matrix[i][j] = (int)std::round(std::sqrt(dx * dx + dy * dy));
                }
            }
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
