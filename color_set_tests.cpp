#include "SubsetWT.hpp"
#include "RRR_generalization.hpp"
#include "SplitStructure.hpp"
#include "BitMagic.hpp"
#include "SDSL_WT.hpp"   

#include <chrono>
#include <fstream>

int32_t readInt32LE(ifstream &file) {
    uint8_t bytes[4];
    file.read(reinterpret_cast<char*>(bytes), 4);
    return static_cast<int32_t>(bytes[0]) |
           (static_cast<int32_t>(bytes[1]) << 8) |
           (static_cast<int32_t>(bytes[2]) << 16) |
           (static_cast<int32_t>(bytes[3]) << 24);
}

int64_t current_time_micros(){
    return (std::chrono::duration_cast< microseconds >(high_resolution_clock::now().time_since_epoch())).count();
}

vector<pair<int, int>> load_queries() {
    int64_t total_time_micros = 0; 
    int64_t n_queries = 0;
    // load queries 
    vector<pair<int, int>> number_pairs;
    ifstream file("output.txt");
    if (!file) {
        cerr << "Error opening file!" << endl;
        return number_pairs;
    }

    
    string line;

    while (getline(file, line)) {
        stringstream ss(line);
        int a, b;
        char comma;

        if (ss >> a >> comma >> b) { // Read two integers separated by a comma
            number_pairs.emplace_back(a, b);
        }
    }

    file.close();
    cout << "Loaded " << number_pairs.size() << " pairs from file:\n";
    return number_pairs;
}

vector<vector<int64_t>> load_color_sets() {
    string filename = "/scratch/project_2014447/coli3682-binary-color-dump.bin";
    ifstream file(filename, ios::binary);
    vector<vector<int64_t>> colorSets;

    if (!file) {
        cerr << "Error: Could not open file " << filename << endl;
        return colorSets;
    }

    

    while (file.peek() != EOF) {  // Continue until end of file
        int32_t setSize = readInt32LE(file);  // Read size of the color set
        if (file.eof()) break;  // Handle premature EOF

        vector<int64_t> colorSet;
        colorSet.reserve(setSize);  // Optimize memory allocation
        
        for (int i = 0; i < setSize; i++) {
            int32_t color = readInt32LE(file);
            colorSet.push_back(color);
        }
        
        colorSets.push_back(move(colorSet));  // Move to avoid unnecessary copies
    }

    file.close();

    // Print the first few color sets for verification
    cout << "Read " << colorSets.size() << " color sets.\n";
    for (size_t i = 0; i < min(colorSets.size(), size_t(2)); i++) { // Print first 3 sets
        // cout << "Set " << i + 1 << " (size: " << colorSets[i].size() << "): ";
        for (size_t j = 0; j < min(colorSets[i].size(), size_t(10)); j++) { // Print first 10 colors
            cout << colorSets[i][j] << " ";
        }
        // for (size_t j = 0; j < colorSets[i].size(); j++) {
        //     cout << colorSets[i][j] << " ";
        // }
        cout << "...\n";
    }
    cout << endl;
    return colorSets;
}

vector<int64_t> randomize_1000() {
    vector<int64_t> queries(1000);
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(1, 558404);
    while (queries.size() < 1000) {
        int a = dist(gen);
        queries.push_back(static_cast<int64_t>(a));
    }
    return queries;
}

int main() {

    // Define the types of the four main variants   
    typedef SubsetWT<SDSL_WT<sdsl::wt_hutu<>, 4>, SDSL_WT<sdsl::wt_hutu<>, 3>> nested_wt_t;
    typedef SubsetWT<RRR_Generalization<4>, RRR_Generalization<3>> rrr_generalization_t;
    typedef SubsetWT<SplitStructure<4>, SplitStructure<3>> split_t;
    typedef SubsetWT<BitMagic<4>, BitMagic<3>> bitmagic_t;

    // Load color sets
    vector<vector<int64_t>> colorSets = load_color_sets();
    
    int64_t t0 = current_time_micros();
    nested_wt_t sswt(colorSets, 7000);
    int64_t t1 = current_time_micros();
    cout << "Tree building time: " << (double)(t1-t0) << " us" << endl;
    cout << "Size: " << sswt.size_in_bytes() << endl;

    return 0;
}