#include "tsp_branch_and_bound.h"
#include <queue>
#include <deque>

TSPResult tsp_branch_and_bound(const std::vector<std::vector<int>>& matrix, int search_mode, int initial_ub) {
    int n = matrix.size();
    TSPResult best_res;
    best_res.cost = initial_ub; // To jest nasze UB (z RNN lub nieskończoność)
    //best_res.nodes_visited = 0;

    // Struktura pomocnicza węzła
    struct Node {
        std::vector<int> path;
        std::vector<bool> visited;
        int current_cost;
        int level;

        // Dla Best-First (mode 3) priorytetem jest teraz sam koszt aktualny
        bool operator>(const Node& other) const {
            return current_cost > other.current_cost;
        }
    };

    Node root;
    root.path = {0};
    root.visited.assign(n, false);
    root.visited[0] = true;
    root.current_cost = 0;
    root.level = 1;

    std::deque<Node> container;
    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> pq;

    if (search_mode == 3) pq.push(root);
    else container.push_back(root);

    while (search_mode == 3 ? !pq.empty() : !container.empty()) {
        Node current;

        if (search_mode == 3) {
            current = pq.top(); pq.pop();
        } else if (search_mode == 2) { // DFS
            current = container.back(); container.pop_back();
        } else { // BFS
            current = container.front(); container.pop_front();
        }

        //best_res.nodes_visited++;

        // KLUCZOWY MOMENT: Odcinanie tylko na podstawie UB (kosztu)
        if (current.current_cost >= best_res.cost) continue;

        if (current.level == n) {
            int final_cost = current.current_cost + matrix[current.path.back()][0];
            if (final_cost < best_res.cost) {
                best_res.cost = final_cost;
                best_res.path = current.path;
                best_res.path.push_back(0);
            }
            continue;
        }

        for (int next = 0; next < n; next++) {
            if (!current.visited[next]) {
                int next_cost = current.current_cost + matrix[current.path.back()][next];

                // Dodajemy tylko jeśli nowa ścieżka nie jest już droższa od UB
                if (next_cost < best_res.cost) {
                    Node child = current;
                    child.visited[next] = true;
                    child.path.push_back(next);
                    child.current_cost = next_cost;
                    child.level++;

                    if (search_mode == 3) pq.push(child);
                    else container.push_back(child);
                }
            }
        }
    }
    return best_res;
}