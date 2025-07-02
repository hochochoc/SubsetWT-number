#include "SubsetWT.hpp"
#include "RRR_generalization.hpp"
#include "SplitStructure.hpp"
#include "BitMagic.hpp"
#include "SDSL_WT.hpp"   
#include "SuccintPerm.hpp"

int64_t current_time_micros(){
    return (std::chrono::duration_cast< microseconds >(high_resolution_clock::now().time_since_epoch())).count();
}

template <typename Func>
long long time_function(Func f) {
    auto start = current_time_micros();
    f();
    auto end = current_time_micros();
    return end-start;
}

int64_t current_time_nanos(){
    return (std::chrono::duration_cast< nanoseconds >(high_resolution_clock::now().time_since_epoch())).count();
}


int main(){
    // Define the types of the four main variants   
    typedef SubsetWT<SDSL_WT<sdsl::wt_int<>, 4>, SDSL_WT<sdsl::wt_int<>, 3>, SuccinctPerm> nested_wt_t;
    typedef SubsetWT<RRR_Generalization<4>, RRR_Generalization<3>, SuccinctPerm> rrr_generalization_t;
    typedef SubsetWT<SplitStructure<4>, SplitStructure<3>, SuccinctPerm> split_t;
    typedef SubsetWT<BitMagic<4>, BitMagic<3>, SuccinctPerm> bitmagic_t;

    // Demonstrate using the split variant

    // vector<vector<char>> sets = {{'A','B'},{'B','C','D'},{'A','C','D'},{'B','C','D'},{'A','B'}};
    // split_t sswt(sets);

    // int64_t pos = 4; // Rank up to but not including this index
    // char symbol = 'B';
    // cout << "rank(" << pos << "," << symbol << ") = " << sswt.rank(pos, symbol) << endl;

    // numbers
    vector<vector<int64_t>> sets = {{1, 2, 3}, {2, 3, 4}, {1, 2, 5}, {5}, {10}, {1, 4}, {11, 12}, {5, 10, 11}, {1, 3, 5}};
    // vector<vector<int64_t>> new_sets = {{1, 2, 3}, {1, 2, 5}, {1, 3, 5}, {1, 4}, {2, 3, 4}, {5}, {5, 10, 11}, {10}, {11, 12}};
    vector<int64_t> perm = {0, 5, 1, 9, 12, 2, 3, 8, 6, 7, 10, 11, 4, 11}; // 0-based indices

    SuccinctPerm p = SuccinctPerm(perm); // permutation of colors [orginal color code 1, orginal color code 2, ...] 0-based index

    vector<vector<int64_t>> reorderedColorSets;
    reorderedColorSets.resize(sets.size());

    for (size_t i = 0; i < sets.size(); i++) {
        vector<int64_t>& originalColorSet = sets[i];
        vector<int64_t>& reorderedSet = reorderedColorSets[i];

        reorderedSet.reserve(originalColorSet.size());

        std::transform(
            originalColorSet.begin(), originalColorSet.end(),
            std::back_inserter(reorderedSet),
            [&p](int64_t code) { return p.inverse(code); }
        );

        std::sort(reorderedSet.begin(), reorderedSet.end());
    }

    nested_wt_t sswt(reorderedColorSets, perm, 13);

    int64_t pos = 5; // Rank up to but not including this index最后
    int64_t symbol = 2;
    cout << "rank(" << pos << "," << symbol << ") = " << sswt.rank(pos, symbol) << endl;



    vector<int64_t> union_set = sswt.union_range(1, 2);
    for (const auto& val : union_set) {
        cout << val << " "; 
    }
    cout << endl;

    vector<int64_t> flat_union_set = sswt.flat_union(1, 2);
    for (const auto& val : flat_union_set) {
        cout << val << " "; 
    }
    cout << endl;
    assert (union_set==flat_union_set);

    vector<int64_t> extract = sswt.extract_set(1);
    for (const auto& val : extract) {
        cout << val << " "; 
    }
    cout << endl;

    vector<int64_t> intersection_set = sswt.intersect(2, 6);
    for (const auto& val : intersection_set) {
        cout << val << " "; 
    }
    cout << endl;

    vector<int64_t> flat_i_set = sswt.flat_intersect_two(2, 6);
    for (const auto& val : flat_i_set) {
        cout << val << " "; 
    }
    cout << endl;

    assert (intersection_set == flat_i_set);
    // cout << sswt.size_in_bytes() << endl;
    // vector<int64_t> perm(50000000);
    // // Fill perm with 0, 1, 2, ..., 49999999
    // for (int64_t i = 0; i < 50000000; ++i)
    //     perm[i] = i;

    // // Shuffle perm if you want
    // cout << perm[2] << endl;

    // SuccinctPerm sp(perm);

    // // Get value at position 30000
    // cout << "Value at position 30000: " << sp.access(2) << endl;

    // // Get position of value 30000 (inverse permutation)
    // cout << "Position of value 30000: " << sp.inverse(perm[2]) << endl;

    return 0;
}

// #include <iostream>
// #include <vector>
// #include <sdsl/int_vector.hpp>
// #include <sdsl/wavelet_trees.hpp>

// pair<vector<int64_t>, vector<vector<int64_t>>> load_reordered_colorsets() {
//     string filename = "result.bin";
//     ifstream file(filename, ios::binary);

//     vector<int64_t> perm;
//     vector<vector<int64_t>> terms_per_doc;

//     if (!file) {
//         cerr << "Failed to open file" << endl;
//         return {perm, terms_per_doc};
//     }

//     while (true) {
//         int64_t org_id;
//         file.read(reinterpret_cast<char*>(&org_id), sizeof(org_id));
//         if (file.eof()) break;

//         int64_t terms_len;
//         file.read(reinterpret_cast<char*>(&terms_len), sizeof(terms_len));
//         if (file.eof() || terms_len < 0 || terms_len > 1000000) break; // sanity check

//         // cout << terms_len << endl;

//         vector<int64_t> terms(terms_len);
//         file.read(reinterpret_cast<char*>(terms.data()), terms_len * sizeof(uint64_t));
//         if (!file) break;

//         perm.push_back(org_id);
//         terms_per_doc.push_back(move(terms));
//     }

//     file.close();

//     // Debug print — limit to however many actually loaded
//     for (size_t i = 0; i < 2; ++i) {
//         cout << "Doc " << i << " org_id: " << perm[i] << " terms: ";
//         for (auto t : terms_per_doc[i])
//             cout << t << " ";
//         cout << endl;
//     }

//     cout << "Number of documents: " << perm.size() << endl;

//     return {perm, terms_per_doc};
// }

// int main() {
//     using namespace std;
//     using namespace sdsl;

//     // const size_t size = 50000000;

//     // Create identity permutation 0..size-1
//     const auto& reordered_colorsets = load_reordered_colorsets(); 
//     vector<int64_t> perm = reordered_colorsets.first;
//     // size_t size = perm.size();
    
//     // // Build int_vector from perm
//     // int_vector<> iv(size, 0, bits::hi(size - 1) + 1); // width = 26 bits for ~50M values
//     // for (size_t i = 0; i < size; ++i) {
//     //     iv[i] = perm[i];
//     // }   
//     // cout << endl;
//     // // Build wavelet tree over iv
//     // wt_int<> wt;
//     // construct_im(wt, iv);

//     // cout << "Wavelet tree size in bits: " << wt.size() << "\n";

//     SuccinctPerm sp = SuccinctPerm(perm);


//     // Test access
//     size_t test_pos = 1;
//     uint64_t val = sp.access(test_pos);
//     cout << "Value at position " << test_pos << " = " << val << "\n"; // should print 2

//     // Test inverse using rank/select (inverse of val = position)
//     // For true permutation, val = test_pos, so inverse of val = val
//     // uint64_t rank_val = wt.rank(test_pos + 1, val);
//     // uint64_t select_val = wt.select(rank_val, val);
//     // cout << "Rank of val " << val << " up to position " << test_pos + 1 << " = " << rank_val << "\n";
//     // cout << "Select occurrence " << rank_val << " of val " << val << " = " << select_val << "\n";
//     cout << "Inverse: " << sp.inverse(val) << endl;

//     return 0;
// }

// #include <iostream>
// #include <vector>
// #include <sdsl/wavelet_trees.hpp>
// #include <sdsl/int_vector.hpp>
// #include <cmath>

// using namespace std;
// using namespace sdsl;

// int main() {
//     vector<uint8_t> seq = {2,3,3,2,2,1,1,1,1, 0};
//     int sigma = 4;
//     uint8_t bits = ceil(log2(sigma ? sigma : 2));
//     cout << "bits = " << (int)bits << endl;

//     int_vector<> iv(seq.size(), 0, 8);
//     for (size_t i = 0; i < seq.size(); ++i) {
//         iv[i] = (seq[i]);
//     }

//     cout << "iv size = " << iv.size() << endl;
//     for (auto v : iv) cout << int(v) << " ";
//     cout << endl;

//     wt_blcd<> wt;
//     construct_im(wt, iv);
//     cout << "wt size = " << wt.size() << endl;  // Should be 9

//     cout << "rank(5, 2) = " << wt.rank(5, 2) << endl;  // Check rank
//     for(size_t i=0; i<=9; ++i){
//         cout << "rank(" << i << ", 2) = " << wt.rank(i, 2) << endl;
//     }

//     return 0;
// }
