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
        uint8_t bits = ceil(log2(sigma ? sigma : 2));
        sdsl::int_vector<> iv(seq.size(), 0, bits);
        for (size_t i = 0; i < seq.size(); ++i) {
            iv[i] = (seq[i]);
        }
        construct_im(wt, iv);
    }

    // Rank of symbol in half-open interval [0..pos)
    int64_t rank(int64_t pos, char symbol) const{
        uint64_t ans = wt.rank(pos, symbol); // Translate to ascii for the query
        return ans;
    }

    // Sum of ranks `symbol` and the last symbol in half-open interval [0..pos)
    int64_t rankpair(int64_t pos, char symbol) const{
        // uint64_t ans = wt.rank(pos, '0' + symbol); // Translate to ascii for the query
        // ans += wt.rank(pos, '0' + (sigma-1)); // Last symbol
        auto r1 = wt.rank(pos, symbol);
        auto r2 = wt.rank(pos, (sigma-1));
        return r1+r2;
    }

    // int64_t rankpair(int64_t pos, char symbol) const{
    //     auto r = wt.rank(pos, symbol) + wt.rank(pos, 0);
    //     if (symbol == sigma - 1) {
    //         return r;
    //     } else if (symbol > 0) {
    //         return r + wt.rank(pos, sigma-1);
    //     }
    //     return wt.rank(pos, symbol);
    // }

    size_t size_in_bytes() const{
        return sdsl::size_in_bytes(wt) + sizeof(sigma);
    }
};