#include <iostream>
#include <fstream>
#include <set>
#include <random>

using namespace std;

int main() {
    ofstream file("output.txt");
    if (!file) {
        cerr << "Error opening file!" << endl;
        return 1;
    }

    set<pair<int, int>> unique_pairs;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(1, 558404); // Numbers between 1 and 558404

    while (unique_pairs.size() < 1000) {
        int a = dist(gen);
        int b = dist(gen);
        if (a > b) swap(a, b); // Ensure a < b
        if (a != b) unique_pairs.insert({a, b});
    }

    for (const auto& p : unique_pairs) {
        file << p.first << "," << p.second << "\n";
    }

    file.close();
    cout << "File 'output.txt' generated successfully!" << endl;
    return 0;
}