#include "utils.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>
#include <vector>

#include "tsp_result.h"

// Funkcja pomocnicza do czyszczenia stringów z białych znaków
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) return str;
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

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
        line = trim(line);
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
        else if (line_upper.find("EDGE_WEIGHT_FORMAT") != std::string::npos) {
            if (line_upper.find("LOWER_DIAG_ROW") != std::string::npos) weight_format = "LOWER_DIAG_ROW";
            if (line_upper.find("FULL_MATRIX") != std::string::npos) weight_format = "FULL_MATRIX";
        }
        else if (line_upper.find("EDGE_WEIGHT_SECTION") != std::string::npos ||
                 line_upper.find("NODE_COORD_SECTION") != std::string::npos) {
            break;
        }
    }

    if (dimension == 0) exit(1);
    std::vector<std::vector<int>> matrix(dimension, std::vector<int>(dimension, 0));

    // 2. Wczytywanie danych
    if (is_explicit) {
        if (weight_format == "LOWER_DIAG_ROW") {
            // Obsługa formatu trójkątnego (gr17, gr21)
            for (int i = 0; i < dimension; i++) {
                for (int j = 0; j <= i; j++) {
                    int val;
                    if (!(file >> val)) break;
                    matrix[i][j] = val;
                    matrix[j][i] = val; // Symetria
                }
            }
        } else {
            // Domyślnie FULL_MATRIX (br17)
            for (int i = 0; i < dimension; i++) {
                for (int j = 0; j < dimension; j++) {
                    file >> matrix[i][j];
                }
            }
        }
    } else {
        // Czytamy współrzędne (ulysses, fri26, berlin52)
        std::vector<std::pair<double, double>> coords(dimension);
        for (int i = 0; i < dimension; i++) {
            int id;
            if (!(file >> id >> coords[i].first >> coords[i].second)) break;
        }

        for (int i = 0; i < dimension; i++) {
            for (int j = 0; j < dimension; j++) {
                if (i == j) matrix[i][j] = 0;
                else if (weight_type == "GEO") {
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