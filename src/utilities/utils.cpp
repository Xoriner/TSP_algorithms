#include "utils.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cmath>
#include <vector>
#include <iomanip>

// Funkcja pomocnicza do czyszczenia stringów ze zbędnych białych znaków
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) return str;
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// Przeliczanie współrzędnych geograficznych (dla ulysses16/22)
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

// Główna funkcja TSPLIB - obsługuje .atsp i .tsp
std::vector<std::vector<int>> read_tsplib(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open TSPLIB file: " << filename << std::endl;
        exit(1);
    }

    std::string line;
    int dimension = 0;
    std::string weight_type = "";
    std::string weight_format = "";
    bool explicit_data = false;

    // 1. Parsowanie Nagłówka
    while (std::getline(file, line)) {
        // Usuwamy dwukropki, żeby ujednolicić format (niektóre pliki mają "DIMENSION:10", inne "DIMENSION : 10")
        std::string line_clean = line;
        std::replace(line_clean.begin(), line_clean.end(), ':', ' ');

        std::string line_up = trim(line_clean);
        std::transform(line_up.begin(), line_up.end(), line_up.begin(), ::toupper);

        if (line_up.empty()) continue;

        if (line_up.find("DIMENSION") != std::string::npos) {
            std::stringstream ss(line_up);
            std::string tmp;
            ss >> tmp >> dimension;
        }
        else if (line_up.find("TYPE") != std::string::npos) {
            if (line_up.find("ATSP") != std::string::npos) {
                explicit_data = true;
                weight_format = "FULL_MATRIX"; // ATSP to z definicji pełna macierz
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
            break;
        }
    }

    if (dimension <= 0) {
        std::cerr << "Error: Invalid dimension (" << dimension << ") in file: " << filename << std::endl;
        exit(1);
    }

    std::vector<std::vector<int>> matrix(dimension, std::vector<int>(dimension, 0));

    // 2. Wczytywanie Danych
    if (explicit_data) {
        if (weight_format == "LOWER_DIAG_ROW") {
            for (int i = 0; i < dimension; i++) {
                for (int j = 0; j <= i; j++) {
                    int val;
                    if (!(file >> val)) break;
                    matrix[i][j] = matrix[j][i] = val;
                }
            }
        } else { // FULL_MATRIX (Typowe dla ATSP)
            for (int i = 0; i < dimension; i++) {
                for (int j = 0; j < dimension; j++) {
                    if (!(file >> matrix[i][j])) break;
                }
            }
        }
    } else { // Współrzędne (EUC_2D lub GEO)
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

// Funkcja dla prostych plików tekstowych (n + macierz)
std::vector<std::vector<int>> read_simple_input(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open simple input file: " << filename << std::endl;
        exit(1);
    }
    int n;
    if (!(file >> n)) return {};
    std::vector<std::vector<int>> matrix(n, std::vector<int>(n));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            file >> matrix[i][j];
    return matrix;
}

void print_solution(const TSPResult& result) {
    if (result.path.empty()) {
        std::cout << "No solution found.\n";
        return;
    }
    std::cout << "\nPath (first 20 cities): ";
    for (size_t i = 0; i < std::min((size_t)20, result.path.size()); i++) {
        std::cout << result.path[i] << (i < std::min((size_t)19, result.path.size() - 1) ? " -> " : "");
    }
    if (result.path.size() > 20) std::cout << " ... -> " << result.path.back();

    std::cout << "\nCost: " << result.cost << "\n";
    std::cout << "Time: " << std::fixed << std::setprecision(2) << (double)result.time_ms << " ms\n";
}