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

    // numbers
    vector<vector<int64_t>> sets = {{1, 2, 3}, {2, 3, 4}, {1, 2, 5}, {5}, {10}, {1, 4}, {11, 12}, {5, 10, 11}, {1, 3, 5}};
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

    return 0;
}