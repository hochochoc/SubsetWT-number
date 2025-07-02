#pragma once

#include <sdsl/int_vector.hpp>
#include <sdsl/wavelet_trees.hpp>

using namespace std;

class SuccinctPerm{
private:
    sdsl::wt_int<> inv_wt;
    size_t size;
    
public:
    SuccinctPerm(): size(0) {}
    SuccinctPerm(const vector<int64_t>& perm){
        size = perm.size();
        if (size == 0) return;

        sdsl::int_vector<> inv_iv(size, 0, sdsl::bits::hi(size - 1) + 1);
        for (size_t i=0; i<size; ++i) {
            inv_iv[perm[i]] = i; // input 0-based
        }
        construct_im(inv_wt, inv_iv);
        cout << "Built permutation Done!!" << endl;
    }

    size_t inverse(int64_t val) const {
        assert (val <= inv_wt.size());
        return inv_wt[val];
    }

    size_t length() const {
        return size;
    }

    size_t size_in_bytes() const{
        return sdsl::size_in_bytes(inv_wt) + sizeof(size);
    }
};