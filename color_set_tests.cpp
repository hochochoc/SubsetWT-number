#include "SubsetWT.hpp"
#include "RRR_generalization.hpp"
#include "SplitStructure.hpp"
#include "BitMagic.hpp"
#include "SDSL_WT.hpp"   
#include "SuccintPerm.hpp"

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

int64_t current_time_nanos(){
    return (std::chrono::duration_cast< nanoseconds >(high_resolution_clock::now().time_since_epoch())).count();
}

vector<pair<int, int>> load_queries() {
    int64_t total_time_micros = 0; 
    int64_t n_queries = 0;
    // load queries 
    vector<pair<int, int>> number_pairs;
    ifstream file("union_range_10.txt");
    if (!file) {
        cerr << "Error opening file!" << endl;
        return number_pairs;
    }

    
    string line;

    while (getline(file, line)) {
        stringstream ss(line);
        int a, b;

        if (ss >> a >> b) { // Read two integers separated by a comma
            number_pairs.emplace_back(a, b);
        }
    }

    file.close();
    cout << "Loaded " << number_pairs.size() << " pairs from file.\n";
    return number_pairs;
}

vector<pair<int, int>> load_queries_50() {
    int64_t total_time_micros = 0; 
    int64_t n_queries = 0;
    // load queries 
    vector<pair<int, int>> number_pairs;
    ifstream file("union_range_1000.txt");
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
    // string filename = "output.bin";
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
    int64_t total_items = 0;
    for (size_t i = 0; i < colorSets.size(); i++) { // Print first 3 sets
        if (i==0) {
            for (size_t j = 0; j < min(colorSets[i].size(), size_t(10)); j++) { // Print first 10 colors
                cout << colorSets[i][j] << " ";
            }
            cout << endl;
        }

        // cout << (colorSets[i].size()) << endl;
        // cout << "Set " << i + 1 << " (size: " << colorSets[i].size() << "): ";
        // for (size_t j = 0; j < min(colorSets[i].size(), size_t(10)); j++) { // Print first 10 colors
        //     cout << colorSets[i][j] << " ";
        // }
        // for (size_t j = 0; j < colorSets[i].size(); j++) {
        //     cout << colorSets[i][j] << " ";
        // }
        total_items += colorSets[i].size(); 
    }
    cout << "total items: " << total_items << endl;
    return colorSets;
}

vector<int64_t> randomize_1000() {
    unordered_set<int64_t> unique_queries;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(1, 558404);
    while (unique_queries.size() < 1000) {
        unique_queries.insert(dist(gen));
    }
    return vector<int64_t>(unique_queries.begin(), unique_queries.end());
}


template <typename Func>
long long time_function(Func f) {
    auto start = current_time_nanos();
    f();
    auto end = current_time_nanos();
    return end-start;
}

pair<vector<int64_t>, vector<vector<int64_t>>> load_reordered_colors() {
    string filename = "/scratch/project_2014447/result.bin";
    // string filename = "result.bin";

    ifstream file(filename, ios::binary);

    vector<int64_t> perm;
    vector<vector<int64_t>> colorSets;

    if (!file) {
        cerr << "Failed to open file" << endl;
        return {perm, colorSets};
    }

    // Read number of docs (colors)
    uint64_t num_docs = 0;
    file.read(reinterpret_cast<char*>(&num_docs), sizeof(num_docs));
    if (!file) {
        cerr << "Failed to read doc count" << endl;
        return {perm, colorSets};
    }

    // Resize vector to hold all org_ids
    perm.resize(num_docs);

    // Read org_ids
    file.read(reinterpret_cast<char*>(perm.data()), num_docs * sizeof(uint64_t));
    if (!file) {
        cerr << "Failed to read org_ids" << endl;
        perm.clear();
        return {perm, colorSets};
    }

    // // Read postings lists (colors)
    // while (file.peek() != EOF) {
    //     uint64_t num_colors = 0;
    //     file.read(reinterpret_cast<char*>(&num_colors), sizeof(num_colors));
    //     if (!file) {
    //         break; // graceful end-of-file or error
    //     }

    //     vector<int64_t> colors;
    //     colors.resize(num_colors);  // << this is the fix

    //     file.read(reinterpret_cast<char*>(colors.data()), num_colors * sizeof(uint64_t));
    //     if (!file) {
    //         cerr << "Failed to read colors" << endl;
    //         perm.clear();
    //         colorSets.clear();
    //         return {perm, colorSets};
    //     }

    //     colorSets.push_back(move(colors));
    // }

    file.close();

    cout << "Number of colors: " << perm.size() << endl;
    cout << "Number of colorSets: " << colorSets.size() << endl;

    return {perm, colorSets};
}

vector<int64_t> convert(vector<int64_t> perm, vector<int64_t> reorderColors) {
    vector<int64_t> orginalColorSet;
    orginalColorSet.reserve(reorderColors.size());
    for (int64_t c : reorderColors) {
        int64_t orgColorCode = perm[c];
        orginalColorSet.push_back(orgColorCode);
    }

    std::sort(orginalColorSet.begin(), orginalColorSet.end());
    return orginalColorSet;
}

int main() {

    // // Define the types of the four main variants   
    typedef SubsetWT<SDSL_WT<sdsl::wt_hutu<>, 4>, SuccinctPerm> nested_wt_t;
    typedef SubsetWT<RRR_Generalization<4>, SuccinctPerm> rrr_generalization_t;
    // typedef SubsetWT<SplitStructure<4>, SplitStructure<3>, SuccinctPerm> split_t;
    // typedef SubsetWT<BitMagic<4>, BitMagic<3>, SuccinctPerm> bitmagic_t;


    vector<vector<int64_t>> colorsets = load_color_sets(); // orginal color sets
    pair<vector<int64_t>, vector<vector<int64_t>>> reordered = load_reordered_colors(); 
    vector<int64_t> perm = reordered.first;
    SuccinctPerm p = SuccinctPerm(perm); // permutation of colors [orginal color code 1, orginal color code 2, ...] 0-based index

    // new subsets after reordering the colors
    vector<vector<int64_t>> reorderedColorSets;
    reorderedColorSets.resize(colorsets.size());

    for (size_t i = 0; i < colorsets.size(); i++) {
        vector<int64_t>& originalColorSet = colorsets[i];
        vector<int64_t>& reorderedSet = reorderedColorSets[i];

        reorderedSet.reserve(originalColorSet.size());

        std::transform(
            originalColorSet.begin(), originalColorSet.end(),
            std::back_inserter(reorderedSet),
            [&p](int64_t code) { return p.inverse(code); }
        );

        std::sort(reorderedSet.begin(), reorderedSet.end());
    }

    vector<int64_t> convertBack = convert(perm, reorderedColorSets[0]);

    assert(convertBack == colorsets[0]);

    cout << "Num of k-mers: " << reorderedColorSets.size() << endl;

    int64_t t0 = current_time_micros();
    nested_wt_t sswt(reorderedColorSets, perm, perm.size());
    int64_t t1 = current_time_micros();
    cout << "Tree building time: " << (double)(t1-t0)/1000 << " ms" << endl;
    cout << "Size: " << sswt.size_in_bytes() << endl;

    vector<pair<int, int>> queries = load_queries();
    uint64_t total_extraction_time = 0;
    std::vector<std::pair<size_t, uint64_t>> extraction_data_points; // (result size, time in microseconds)
    for (const auto& p : queries) {
        t0 = current_time_micros();
        vector<int64_t> result = sswt.extract_set(p.first);
        t1 = current_time_micros();
        assert (reorderedColorSets[p.first-1] == result);

        uint64_t time_micros = (t1 - t0); // convert to microseconds
        total_extraction_time += time_micros;
        extraction_data_points.emplace_back(result.size(), time_micros);
    }
    cout << "Average extraction time: " << total_extraction_time/queries.size() << endl;
    std::ofstream eout("extract_plot_data_hutu_all_reordered.csv");
    eout << "result_size,time_micros\n";
    for (const auto& [size, time] : extraction_data_points) {
             eout << size << "," << time << "\n";
    }
    eout.close();

    // // Intersect and union
    uint64_t total_i_time = 0;
    std::vector<std::pair<size_t, uint64_t>> intersection_data_points; // (result size, time in microseconds)
    for (const auto& p : queries) {
        t0 = current_time_micros();
        vector<int64_t> result = sswt.intersect(p.first, p.second);
        t1 = current_time_micros();

        assert(sswt.flat_intersect_two(p.first, p.second) == result);

        uint64_t time_micros = (t1 - t0); // convert to microseconds
        total_i_time += time_micros;
        intersection_data_points.emplace_back(result.size(), time_micros);
    }
    cout << "Average intersection time: " << total_i_time/queries.size() << endl;
    std::ofstream out("intersect_plot_data_hutu_all_reordered.csv");
    out << "result_size,time_micros\n";
    for (const auto& [size, time] : intersection_data_points) {
        out << size << "," << time << "\n";
    }
    out.close();

    uint64_t total_ur_time = 0;
    std::vector<std::pair<size_t, uint64_t>> ur_data_points; // (result size, time in microseconds)
    for (const auto& p : queries) {
        t0 = current_time_micros();
        vector<int64_t> result = sswt.union_range(p.first, p.second);
        t1 = current_time_micros();

        assert(sswt.flat_union(p.first, p.second) == result);

        uint64_t time_micros = (t1 - t0); // convert to microseconds
        total_ur_time += time_micros;
        ur_data_points.emplace_back(result.size(), time_micros);
    }
    cout << "Average union range time: " << total_ur_time/queries.size() << endl;
    std::ofstream urout("ur_plot_data_hutu_all_reordered.csv");
    urout << "result_size,time_micros\n";
    for (const auto& [size, time] : ur_data_points) {
        urout << size << "," << time << "\n";
    }
    urout.close();

    return 0;
}
