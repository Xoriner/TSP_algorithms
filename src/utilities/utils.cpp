#include "utils.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>
#include <vector>

#include "tsp_result.h"

// Funkcja pomocnicza do czyszczenia stringów z białych znaków (trim)
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) return str;
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// Funkcja dla prostych plików tekstowych (np. z zajęć)
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

// Przeliczanie współrzędnych geograficznych (wymagane dla plików ulysses16/22)
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

// Główna funkcja obsługująca formaty TSPLIB (.atsp i .tsp)
std::vector<std::vector<int>> read_tsplib(std::string filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        exit(1);
    }

    std::string line;
    int dimension = 0;
    std::string weight_type = "";
    std::string weight_format = "";
    bool explicit_data = false;

    // --- 1. Parsowanie Nagłówka ---
    while (std::getline(file, line)) {
        std::string line_up = trim(line);
        std::transform(line_up.begin(), line_up.end(), line_up.begin(), ::toupper);

        if (line_up.empty()) continue;

        if (line_up.find("DIMENSION") != std::string::npos) {
            size_t pos = line_up.find(":");
            if (pos != std::string::npos) {
                dimension = std::stoi(trim(line_up.substr(pos + 1)));
            } else {
                // Obsługa formatu bez dwukropka
                std::stringstream ss(line_up);
                std::string tmp;
                ss >> tmp >> dimension;
            }
        }
        else if (line_up.find("EDGE_WEIGHT_TYPE") != std::string::npos) {
            if (line_up.find("GEO") != std::string::npos) weight_type = "GEO";
            else if (line_up.find("EUC_2D") != std::string::npos) weight_type = "EUC_2D";
            else if (line_up.find("EXPLICIT") != std::string::npos) explicit_data = true;
        }
        else if (line_up.find("EDGE_WEIGHT_FORMAT") != std::string::npos) {
            if (line_up.find("LOWER_DIAG_ROW") != std::string::npos) weight_format = "LOWER_DIAG_ROW";
            else if (line_up.find("FULL_MATRIX") != std::string::npos) weight_format = "FULL_MATRIX";
        }
        else if (line_up.find("EDGE_WEIGHT_SECTION") != std::string::npos ||
                 line_up.find("NODE_COORD_SECTION") != std::string::npos) {
            break; // Rozpoczynamy wczytywanie danych numerycznych
        }
    }

    if (dimension <= 0) {
        std::cerr << "Error: Could not determine dimension of " << filename << std::endl;
        exit(1);
    }

    std::vector<std::vector<int>> matrix(dimension, std::vector<int>(dimension, 0));

    // --- 2. Wczytywanie Danych ---
    if (explicit_data) {
        if (weight_format == "LOWER_DIAG_ROW") {
            // Obsługa gr17.tsp, gr21.tsp (macierze symetryczne trójkątne)
            for (int i = 0; i < dimension; i++) {
                for (int j = 0; j <= i; j++) {
                    int val;
                    if (!(file >> val)) break;
                    matrix[i][j] = val;
                    matrix[j][i] = val;
                }
            }
        } else {
            // Obsługa br17.atsp, ftv33.atsp, ftv35.atsp itd. (macierze pełne)
            for (int i = 0; i < dimension; i++) {
                for (int j = 0; j < dimension; j++) {
                    if (!(file >> matrix[i][j])) break;
                }
            }
        }
    } else {
        // Obsługa ulysses16.tsp, ulysses22.tsp, fri26.tsp (współrzędne)
        std::vector<std::pair<double, double>> coords(dimension);
        for (int i = 0; i < dimension; i++) {
            int id;
            if (!(file >> id >> coords[i].first >> coords[i].second)) break;
        }

        for (int i = 0; i < dimension; i++) {
            for (int j = 0; j < dimension; j++) {
                if (i == j) {
                    matrix[i][j] = 0;
                } else if (weight_type == "GEO") {
                    matrix[i][j] = calc_geo_dist(coords[i].first, coords[i].second, coords[j].first, coords[j].second);
                } else {
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
        if (i != result.path.size() - 1) std::cout << " -> ";
    }
    std::cout << "\n";
}