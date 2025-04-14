#include "SubsetWT.hpp"
#include "RRR_generalization.hpp"
#include "SplitStructure.hpp"
#include "BitMagic.hpp"
#include "SDSL_WT.hpp"   

int main(){

    // Define the types of the four main variants   
    typedef SubsetWT<SDSL_WT<sdsl::wt_hutu<>, 4>, SDSL_WT<sdsl::wt_hutu<>, 3>> nested_wt_t;
    typedef SubsetWT<RRR_Generalization<4>, RRR_Generalization<3>> rrr_generalization_t;
    typedef SubsetWT<SplitStructure<4>, SplitStructure<3>> split_t;
    typedef SubsetWT<BitMagic<4>, BitMagic<3>> bitmagic_t;

    // Demonstrate using the split variant

    // vector<vector<char>> sets = {{'A','B'},{'B','C','D'},{'A','C','D'},{'B','C','D'},{'A','B'}};
    // split_t sswt(sets);

    // int64_t pos = 4; // Rank up to but not including this index
    // char symbol = 'B';
    // cout << "rank(" << pos << "," << symbol << ") = " << sswt.rank(pos, symbol) << endl;

    // numbers
    vector<vector<int64_t>> sets = {{1, 2, 3}, {2, 3, 4}, {1, 2, 5}, {5}, {10}, {1, 4}, {11, 12}, {5, 10, 11}, {1, 3, 5}};
    rrr_generalization_t sswt(sets, 13);


    vector<int64_t> union_set = sswt.union_range(5, 8);
    for (const auto& val : union_set) {
        cout << val << " "; 
    }
    cout << endl;
    cout << sswt.size_in_bytes() << endl;

    return 0;
}