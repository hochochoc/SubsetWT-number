#pragma once

#include <sdsl/bit_vectors.hpp>
#include <sdsl/rank_support_v.hpp>
#include <sdsl/wavelet_trees.hpp>
#include <string>

using namespace std;

// A wrapper for the basic SDSL wavelet tree
// wt_t is the type of the internal wavelet tree
// sigma is the size of the alphabet
template<typename wt_t, int64_t sigma>
class SDSL_WT{
public:

    wt_t wt;

    SDSL_WT(){}

    SDSL_WT(const vector<char>& seq){
        // We have to translate 0..sigma-1 to ascii digits for the SDSL wavelet tree because
        // apparently we can't give a zero-byte because that will terminate the string?
        string ascii;
        for(char c : seq){
            ascii.push_back('0' + c);
        }
        sdsl::construct_im(wt, ascii.c_str(), 1); // 1: file format is a sequence, not a serialized sdsl object
    }

    // Rank of symbol in half-open interval [0..pos)
    int64_t rank(int64_t pos, char symbol) const{
        uint64_t ans = wt.rank(pos, '0' + symbol); // Translate to ascii for the query
        return ans;
    }

    // Sum of ranks `symbol` and the last symbol in half-open interval [0..pos)
    int64_t rankpair(int64_t pos, char symbol) const{
        uint64_t ans = wt.rank(pos, '0' + symbol); // Translate to ascii for the query
        ans += wt.rank(pos, '0' + (sigma-1)); // Last symbol
        return ans;
    }

    size_t size_in_bytes() const{
        return sdsl::size_in_bytes(wt) + sizeof(sigma);
    }
};