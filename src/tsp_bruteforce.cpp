#include <iostream>
#include <vector>
#include <algorithm>
#include <limits>

using namespace std;

int tsp_bruteforce(const vector<vector<int>>& dist) {
    int n = dist.size();

    vector<int> perm;
    for(int i = 1; i < n; i++)
        perm.push_back(i);

    // for safety max value of int
    int best_cost = numeric_limits<int>::max();

    do {
        int cost = 0;
        int prev = 0;

        for(int v : perm) {
            cost += dist[prev][v];
            prev = v;
        }

        cost += dist[prev][0];

        best_cost = min(best_cost, cost);

    } while(next_permutation(perm.begin(), perm.end()));

    return best_cost;
}

int main() {

    vector<vector<int>> dist = {
        {0,10,15,20},
        {10,0,35,25},
        {15,35,0,30},
        {20,25,30,0}
    };

    string a;
    cout << "Best cost: " << tsp_bruteforce(dist) << endl;
    cin >> a;
}