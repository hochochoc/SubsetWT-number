#include "SubsetWT.hpp"
#include "RRR_generalization.hpp"
#include "SplitStructure.hpp"
#include "BitMagic.hpp"
#include "SDSL_WT.hpp"   
#include "SuccintPerm.hpp"


int main(){
    // Define the types of the four main variants   
    typedef SubsetWT<SDSL_WT<sdsl::wt_int<>, 4>, SDSL_WT<sdsl::wt_int<>, 3>, SuccinctPerm> nested_wt_t;
    typedef SubsetWT<RRR_Generalization<4>, RRR_Generalization<3>, SuccinctPerm> rrr_generalization_t;
    typedef SubsetWT<SplitStructure<4>, SplitStructure<3>, SuccinctPerm> split_t;
    typedef SubsetWT<BitMagic<4>, BitMagic<3>, SuccinctPerm> bitmagic_t;

    static const int64_t CHILD_RIGHT = 1; // '01'
    static const int64_t CHILD_LEFT = 2; // '10'
    static const int64_t CHILD_BOTH = 3; // '11'
    static const int64_t CHILD_ALL = 0; // '?'

    vector<char> split_seq = {CHILD_RIGHT, CHILD_ALL, CHILD_BOTH, CHILD_LEFT, CHILD_LEFT, CHILD_RIGHT, CHILD_RIGHT, CHILD_ALL, CHILD_BOTH, CHILD_LEFT};

    SDSL_WT<sdsl::wt_int<>, 4> children_int = SDSL_WT<sdsl::wt_int<>, 4>(split_seq);
    RRR_Generalization<4> children_rrr = RRR_Generalization<4>(split_seq);

    for (size_t i=1; i<=split_seq.size(); i++) {
        assert(children_int.rankpair(i, CHILD_LEFT) == children_rrr.rankpair(i, CHILD_LEFT));
        assert(children_int.rankpair(i, CHILD_RIGHT) == children_rrr.rankpair(i, CHILD_RIGHT));
        assert(children_int.rank(i, CHILD_ALL) == children_int.rank(i, CHILD_ALL));
        assert(children_int.rank(i, CHILD_BOTH) == children_int.rank(i, CHILD_BOTH));
    }

    return 0;
}