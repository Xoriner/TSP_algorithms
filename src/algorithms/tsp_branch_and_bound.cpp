#include "tsp_branch_and_bound.h"
#include <queue>
#include <deque>
#include <vector>
#include <cstdint>

// Maksymalnie odchudzona struktura węzła
struct Node {
    uint64_t visited_mask;
    int last_v;
    int current_cost;
    int level;
    // Usunięcie std::vector<int> path stąd oszczędza mnóstwo RAMu
    // Ale w zamian w Mode 3 i Mode 1 musimy wiedzieć jak tu dotarliśmy
    // Aby nie komplikować (drzewo ojców), zostawimy małą tablicę statyczną lub vector tylko dla wyników
    std::vector<int> path;

    bool operator>(const Node& other) const {
        return current_cost > other.current_cost;
    }
};

TSPResult tsp_branch_and_bound(const std::vector<std::vector<int>>& matrix, int search_mode, int initial_ub) {
    int n = matrix.size();
    TSPResult best_res;
    best_res.cost = initial_ub;

    // Kolejki
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;
    std::deque<Node> container;

    Node root;
    root.last_v = 0;
    root.level = 1;
    root.current_cost = 0;
    root.visited_mask = 1ULL << 0;
    root.path = {0};

    if (search_mode == 3) pq.push(root);
    else container.push_back(root);

    while (search_mode == 3 ? !pq.empty() : !container.empty()) {
        Node current;
        if (search_mode == 3) {
            current = pq.top(); pq.pop();
        } else if (search_mode == 2) {
            current = container.back(); container.pop_back();
        } else {
            current = container.front(); container.pop_front();
        }

        // Proste odcięcie (tylko to co chciałeś)
        if (current.current_cost >= best_res.cost) continue;

        if (current.level == n) {
            int final_cost = current.current_cost + matrix[current.last_v][0];
            if (final_cost < best_res.cost) {
                best_res.cost = final_cost;
                best_res.path = current.path;
                best_res.path.push_back(0);
            }
            continue;
        }

        for (int next = 0; next < n; next++) {
            // Sprawdzenie bitmaski (czy miasto 'next' nieodwiedzone)
            if (!((current.visited_mask >> next) & 1)) {
                int next_cost = current.current_cost + matrix[current.last_v][next];

                if (next_cost < best_res.cost) {
                    Node child;
                    child.last_v = next;
                    child.level = current.level + 1;
                    child.current_cost = next_cost;
                    child.visited_mask = current.visited_mask | (1ULL << next);

                    // Niestety, żeby zwrócić ścieżkę w TSPResult, musimy ją tu kopiować
                    // Ale bez Lower Bound to jedyny sposób na rzetelny wynik
                    child.path = current.path;
                    child.path.push_back(next);

                    if (search_mode == 3) pq.push(child);
                    else container.push_back(child);
                }
            }
        }
    }
    return best_res;
}