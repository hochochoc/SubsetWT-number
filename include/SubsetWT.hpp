#pragma once 

#include <vector>
#include <utility>
#include <iostream>
#include <optional>
#include <stack>

using namespace std;

template<typename base4_rank_t, typename base3_rank_t>
class SubsetWT{

public:

    static const int64_t ROOT_NONE = 0; // '00'
    static const int64_t ROOT_RIGHT = 1; // '01'
    static const int64_t ROOT_LEFT = 2; // '10'
    static const int64_t ROOT_BOTH = 3; // '11'

    static const int64_t CHILD_RIGHT = 0; // '01'
    static const int64_t CHILD_LEFT = 1; // '10'
    static const int64_t CHILD_BOTH = 2; // '11'

    vector<int64_t> alphabet;
    vector<int64_t> char_to_idx; // Vector of length 256 mapping characters to their indices in the alphabet
    
    base4_rank_t root;

    // children[0] and children[1] are the left and right children of the root
    // The children of children[i] are at children[2*i+2] and children[2*i + 3]
    vector<optional<base3_rank_t>> children;

    // Alphabet intervals of child nodes
    vector<optional<pair<int64_t, int64_t>>> child_intervals;    


private:

    constexpr int64_t get_left_child_idx(int64_t child_idx) const{
        return 2*child_idx+2;
    }

    constexpr int64_t get_right_child_idx(int64_t child_idx) const{
        return 2*child_idx+2 + 1;
    }

    // Helper function used in the constructor
    void init_alphabet(const vector<vector<int64_t>>& sets, int64_t total){
        int64_t alphabet_size = total;
        vector<bool> alphabet_bitmap(alphabet_size, 0); // Temporary bitmap to mark the distinct characters
        char_to_idx.resize(alphabet_size, -1);

        for(const vector<int64_t>& v : sets){
            for(int64_t c : v) {
                alphabet_bitmap[c] = 1;
            }
        }

        for(int64_t c = 0; c < alphabet_size; c++){
            if(alphabet_bitmap[c]){
                char_to_idx[c] = alphabet.size();
                alphabet.push_back(c);
            }
        }
    
    }

    // Helper function used in the constructor
    // Alphabet must be initialized before calling
    // [start, end) is a half-open interval
    void init_children_recursion(int64_t child_idx, int64_t start, int64_t end, const vector<vector<int64_t>>& sets, vector<int64_t>&& sets_in_this_child){

        if(end - start <= 1) return; // Alphabet is singleton or empty

        child_intervals[child_idx] = {start,end};

        int64_t middle_char = alphabet[(start + end)/2];
        // [start, midpoint) go left, [midpoint, end) go right

        // Set indexes that go to left and to right
        vector<int64_t> sets_to_left, sets_to_right;

        vector<char> split_seq; // 0 = left, 1 = right, 2 = both

        // Split the alphabet
        for(int64_t i : sets_in_this_child){
            bool has_left = false;
            bool has_right = false;
            for(int64_t c : sets[i]){
                if(c >= alphabet[start] && c < middle_char){
                    has_left = true;
                } else if(c >= middle_char && (end == alphabet.size() || c < alphabet[end])){
                    has_right = true;
                }
            }
            if(has_left & !has_right) split_seq.push_back(CHILD_LEFT);
            else if(!has_left & has_right) split_seq.push_back(CHILD_RIGHT);
            else split_seq.push_back(CHILD_BOTH);

            if(has_left) sets_to_left.push_back(i);
            if(has_right) sets_to_right.push_back(i);
        }

        vector<int64_t>().swap(sets_in_this_child); // Free memory

        // Construct the base-3 rank support
        children[child_idx] = base3_rank_t(split_seq);

        // Recurse to children
        int64_t left_idx = get_left_child_idx(child_idx);
        int64_t right_idx = get_right_child_idx(child_idx);
        init_children_recursion(left_idx , start          , (start + end)/2, sets, std::move(sets_to_left));
        init_children_recursion(right_idx, (start + end)/2, end            , sets, std::move(sets_to_right));
    }

    // Helper function used in the constructor
    // Alphabet must be initialized before calling
    void init_tree(const vector<vector<int64_t>>& sets){
        int64_t sigma = alphabet.size();
        int64_t middle_char = alphabet[sigma/2];

        // Initialize the root
        vector<char> root_split_seq;
        vector<int64_t> sets_to_left_child, sets_to_right_child;
        for(int64_t i = 0; i < sets.size(); i++){
            bool has_left = false;
            bool has_right = false;
            for(int64_t c : sets[i]){
                if(c < middle_char) has_left = true;
                else has_right = true;
            }

            if(!has_left && !has_right) root_split_seq.push_back(ROOT_NONE);
            else if(has_left && !has_right) root_split_seq.push_back(ROOT_LEFT);
            else if(!has_left && has_right) root_split_seq.push_back(ROOT_RIGHT);
            else if(has_left && has_right) root_split_seq.push_back(ROOT_BOTH);

            if(has_left) sets_to_left_child.push_back(i);
            if(has_right) sets_to_right_child.push_back(i);
        }

        // what base4_rank_t does?
        root = base4_rank_t(root_split_seq);

        // Initialize space for the children
        child_intervals.resize(2*sigma);
        children.resize(2*sigma);

        // Initialize the children
        init_children_recursion(0, 0, sigma/2, sets, std::move(sets_to_left_child)); // Left child of root
        init_children_recursion(1, sigma/2, sigma, sets, std::move(sets_to_right_child)); // Right child of root
    }

    char get_root_sym(int64_t pos) const {
        if (pos < 0) {
            throw std::out_of_range("pos is out of range");
        }

        bool has_left = root.rankpair(pos, ROOT_LEFT) > root.rankpair(pos-1, ROOT_LEFT);

        bool has_right = root.rankpair(pos, ROOT_RIGHT) > root.rankpair(pos-1, ROOT_RIGHT);

        if (has_left && has_right) return ROOT_BOTH;
        if (has_left) return ROOT_LEFT; 
        if (has_right) return ROOT_RIGHT;
     
        return ROOT_NONE;
    }

    char get_child_sym(int64_t pos, int64_t child_idx) const {
        if (pos < 0) {
            throw out_of_range("pos is out of range");
        }
        if (child_idx >= children.size() || !children[child_idx]) {
            return CHILD_BOTH;
        } 

        const auto& child_rank = *children[child_idx];

        bool has_left = child_rank.rankpair(pos, CHILD_LEFT) > child_rank.rankpair(pos-1, CHILD_LEFT);
        bool has_right = child_rank.rankpair(pos, CHILD_RIGHT) > child_rank.rankpair(pos-1, CHILD_RIGHT);

        if (has_left && has_right) return CHILD_BOTH;
        if (has_left) return CHILD_LEFT; 
        if (has_right) return CHILD_RIGHT;
        return CHILD_BOTH;
    }

    void collect_set(int64_t pos, int64_t child_idx, int64_t start, int64_t end, vector<int64_t>& result_set) const {
        // Traverse deeper into the children 
        // how big of children
        if (child_idx >= children.size() || !children[child_idx]) {  
            result_set.insert(result_set.end(), alphabet.begin() + start, alphabet.begin() + end);
            return;
        }
        int64_t x = pos;
    
        const auto& interval = *child_intervals[child_idx];
        int64_t left = interval.first;
        int64_t right = interval.second; // get child range 

            // what is child_intervals? 
        char child_sym = get_child_sym(x, child_idx);

        if (child_sym == CHILD_LEFT || child_sym == CHILD_BOTH) {
            collect_set(children[child_idx]->rankpair(x, CHILD_LEFT), get_left_child_idx(child_idx), start, (left+right)/2, result_set);
        }
        if (child_sym == CHILD_RIGHT || child_sym == CHILD_BOTH) {
            collect_set(children[child_idx]->rankpair(x, CHILD_RIGHT), get_right_child_idx(child_idx), (left+right)/2, end, result_set);
        }
    }

    struct Frame {
        int64_t pos; 
        int64_t child_idx; 
        int64_t start;
        int64_t end;
    };

    void collect_set_stack(int64_t pos, int64_t child_idx, int64_t start, int64_t end, vector<int64_t>& result_set) const {
        
        std::stack<Frame> stk; 
        stk.push({pos, child_idx, start, end});
        // Traverse deeper into the children 
        // how big of children
        while(!stk.empty()) {
            Frame f = stk.top(); stk.pop(); 

            if (f.child_idx >= children.size() || !children[f.child_idx]) {
                result_set.insert(result_set.end(), alphabet.begin() + start, alphabet.begin() + end);
                continue;
            }
            const auto& interval = *child_intervals[child_idx];
            int64_t left = interval.first;
            int64_t right = interval.second; // get child range 

                // what is child_intervals? 
            char child_sym = get_child_sym(f.pos, child_idx);

            if (child_sym == CHILD_LEFT || child_sym == CHILD_BOTH) {
                int64_t new_pos = children[f.child_idx]->rankpair(f.pos, CHILD_RIGHT);
                stk.push({new_pos, get_right_child_idx(f.child_idx), (left+right)/2, f.end});
            }

            if (child_sym == CHILD_RIGHT || child_sym == CHILD_BOTH) {
                int64_t new_pos = children[f.child_idx]->rankpair(f.pos, CHILD_LEFT);
                stk.push({new_pos, get_left_child_idx(child_idx), (left+right)/2, f.end});
            }
        }
    }

    void intersect_helper(int64_t x1, int64_t x2, int64_t child_idx1, int64_t child_idx2, int64_t start, int64_t end, vector<int64_t>& intersection) {

        if ((child_idx1 >= children.size() || !children[child_idx1]) || (child_idx2 >= children.size() || !children[child_idx2])) {
            intersection.insert(intersection.end(), alphabet.begin()+start, alphabet.begin()+end);
            return;
        }

        const auto& interval = *child_intervals[child_idx1];
        int64_t left = interval.first;
        int64_t right = interval.second;

        char child_sym1 = get_child_sym(x1, child_idx1);
        char child_sym2 = get_child_sym(x2, child_idx2);

        if ((child_sym1 == CHILD_LEFT || child_sym1 == CHILD_BOTH) &&
            (child_sym2 == CHILD_LEFT || child_sym2 == CHILD_BOTH)) {
                int64_t new_x1 = children[child_idx1]->rankpair(x1, CHILD_LEFT);
                int64_t new_x2 = children[child_idx2]->rankpair(x2, CHILD_LEFT);

                intersect_helper(new_x1, new_x2, get_left_child_idx(child_idx1), get_left_child_idx(child_idx2), start, (left+right)/2, intersection);
            }
        if ((child_sym1 == CHILD_RIGHT || child_sym1 == CHILD_BOTH) &&
            (child_sym2 == CHILD_RIGHT || child_sym2 == CHILD_BOTH)) {
                int64_t new_x1 = children[child_idx1]->rankpair(x1, CHILD_RIGHT);
                int64_t new_x2 = children[child_idx2]->rankpair(x2, CHILD_RIGHT);

                intersect_helper(new_x1, new_x2, get_right_child_idx(child_idx1), get_right_child_idx(child_idx2), (left+right)/2, end, intersection);
            }
        
    }

    void union_helper(int64_t x1, int64_t x2, int64_t child_idx1, int64_t child_idx2, int64_t start, int64_t end, vector<int64_t>& union_set) {
        if ((child_idx1 >= children.size() || !children[child_idx1]) || (child_idx2 >= children.size() || !children[child_idx2])) {
            union_set.insert(union_set.end(), alphabet.begin()+start, alphabet.begin()+end);
            return;
        }

        const auto& interval = *child_intervals[child_idx1];
        int64_t left = interval.first;
        int64_t right = interval.second;

        char child_sym1 = get_child_sym(x1, child_idx1);
        char child_sym2 = get_child_sym(x2, child_idx2);

        int64_t x1_left = children[child_idx1]->rankpair(x1, CHILD_LEFT);
        int64_t x1_right = children[child_idx1]->rankpair(x1, CHILD_RIGHT);

        int64_t x2_left = children[child_idx2]->rankpair(x2, CHILD_LEFT);
        int64_t x2_right = children[child_idx2]->rankpair(x2, CHILD_RIGHT);
    
        if (child_sym1 == child_sym2) {
            if ((child_sym1 == CHILD_BOTH) || (child_sym1 == CHILD_LEFT))  {
                union_helper(x1_left, x2_left, get_left_child_idx(child_idx1), get_left_child_idx(child_idx2), start, (left+right)/2, union_set);
            } 
            if ((child_sym1 == CHILD_BOTH) || (child_sym1 == CHILD_RIGHT)) {
                union_helper(x1_right, x2_right, get_right_child_idx(child_idx1), get_right_child_idx(child_idx2), (left+right)/2, end, union_set);
            }
        } else {
            if (child_sym1 == CHILD_BOTH) {
                if (child_sym2 == CHILD_LEFT) {
                    union_helper(x1_left, x2_left, get_left_child_idx(child_idx1), get_left_child_idx(child_idx2), start, (left+right)/2, union_set); 
                    collect_set(x1_right, get_right_child_idx(child_idx1), (left+right)/2, end, union_set); 
                } else {// CHILD_RIGHT
                    collect_set(x1_left, get_left_child_idx(child_idx1), start, (left+right)/2, union_set);
                    union_helper(x1_right, x2_right, get_right_child_idx(child_idx1), get_right_child_idx(child_idx2), (left+right)/2, end, union_set);
                }
            } else if (child_sym1 == CHILD_LEFT) {
                if (child_sym2 == CHILD_RIGHT) {
                    collect_set(x1_left, get_left_child_idx(child_idx1), start, (left+right)/2, union_set);
                } else { // CHILD_BOTH
                    union_helper(x1_left, x2_left, get_left_child_idx(child_idx1), get_left_child_idx(child_idx2), start, (left+right)/2, union_set); 
                }
                collect_set(x2_right, get_right_child_idx(child_idx2), (left+right)/2, end, union_set); 
            } else {
                collect_set(x2_left, get_left_child_idx(child_idx2), start, (left+right)/2, union_set);
                if (child_sym2 == CHILD_LEFT) {
                    collect_set(x1_right, get_right_child_idx(child_idx1), (left+right)/2, end, union_set); 
                } else { //CHILD_BOTH
                    union_helper(x1_right, x2_right, get_right_child_idx(child_idx1), get_right_child_idx(child_idx2), (left+right)/2, end, union_set);
                }
            }
            
        }
    }

    pair<vector<int64_t>, vector<int64_t>> extract_syms(int64_t l, int64_t r) const {
        vector<int64_t> left_syms;
        vector<int64_t> right_syms;
        for (int64_t i = l; i <= r; ++i) {
            char sym = get_root_sym(i);
            if (sym == ROOT_BOTH || sym == ROOT_LEFT) {
                left_syms.push_back(i); 
            } 
            if (sym == ROOT_BOTH || sym == ROOT_RIGHT) {
                right_syms.push_back(i);
            }
        }
        return {left_syms, right_syms};
    }

    pair<vector<int64_t>, vector<int64_t>> extract_child_syms(int64_t child_idx, int64_t l, int64_t r) const {
        vector<int64_t> left_child_syms;
        vector<int64_t> right_child_syms;
        for (int64_t i = l; i <= r; ++i) {
            char sym = get_child_sym(i, child_idx);
            if (sym == CHILD_BOTH || sym == CHILD_LEFT) {
                left_child_syms.push_back(i); 
            } 
            if (sym == CHILD_BOTH || sym == CHILD_RIGHT) {
                right_child_syms.push_back(i);
            }
        }
        return {left_child_syms, right_child_syms};
    }

public:

    SubsetWT(){}

    SubsetWT(const vector<vector<int64_t>>& sets, int64_t total){
        init_alphabet(sets, total);
        init_tree(sets);
    }

    // Count of character c in subsets up to pos, not including pos
    int64_t rank(int64_t pos, int64_t c) const{
        int64_t char_idx = char_to_idx[c];
        if(char_idx == -1) return 0; // Character not found

        char root_sym = char_idx < alphabet.size()/2 ? ROOT_LEFT : ROOT_RIGHT;
        int64_t x = root.rankpair(pos, root_sym);

        // Traverse down the tree
        int64_t child_idx = (root_sym == ROOT_LEFT) ? 0 : 1;
        while(child_idx < children.size() && children[child_idx]){ // Child exists
            const auto [left, right] = *child_intervals[child_idx]; // C++17 structured binding
            char child_sym = char_idx < (left + right) / 2 ? CHILD_LEFT : CHILD_RIGHT;

            x = children[child_idx]->rankpair(x, child_sym);
            child_idx = char_idx < (left + right) / 2 ? get_left_child_idx(child_idx) : get_right_child_idx(child_idx);
        }

        return x;
    }

    // return i-th color set as a vector of integers 
    vector<int64_t> extract_set(int64_t pos) const {
        vector<int64_t> result_set; 

        int64_t child_idx = 0; // start at root 
        int64_t start = 0, end = alphabet.size();

        char root_sym = get_root_sym(pos);
        
        int64_t x = pos;
        
        if (root_sym == ROOT_NONE) return result_set;

        if (root_sym == ROOT_LEFT || root_sym == ROOT_BOTH) {
            child_idx = 0; 
            x = root.rankpair(pos, ROOT_LEFT);
            collect_set(x, 0, 0, alphabet.size() / 2, result_set);
        } 
        if (root_sym == ROOT_RIGHT || root_sym == ROOT_BOTH) {
            child_idx = 1; 
            x = root.rankpair(pos, ROOT_RIGHT);
            collect_set(x, 1, alphabet.size() / 2, end, result_set);
        }

        return result_set;
    }

    // return i-th color set as a vector of integers 
    vector<int64_t> extract_set_stack(int64_t pos) const {
        vector<int64_t> result_set; 

        int64_t child_idx = 0; // start at root 
        int64_t start = 0, end = alphabet.size();

        char root_sym = get_root_sym(pos);
        
        int64_t x = pos;
        
        if (root_sym == ROOT_NONE) return result_set;

        if (root_sym == ROOT_LEFT || root_sym == ROOT_BOTH) {
            child_idx = 0; 
            x = root.rankpair(pos, ROOT_LEFT);
            collect_set_stack(x, 0, 0, alphabet.size() / 2, result_set);
        } 
        if (root_sym == ROOT_RIGHT || root_sym == ROOT_BOTH) {
            child_idx = 1; 
            x = root.rankpair(pos, ROOT_RIGHT);
            collect_set_stack(x, 1, alphabet.size() / 2, end, result_set);
        }

        return result_set;
    }

    vector<int64_t> intersect(int64_t pos1, int64_t pos2) {
        vector<int64_t> intersection;

        int64_t start = 0, end = alphabet.size();

        int64_t x1, x2;

        int64_t child_idx1, child_idx2;

        char root_sym1 = get_root_sym(pos1);
        char root_sym2 = get_root_sym(pos2);

        if ((root_sym1 == ROOT_LEFT || root_sym1 == ROOT_BOTH) && 
            (root_sym2 == ROOT_LEFT || root_sym2 == ROOT_BOTH)) {
                child_idx1 = 0;
                child_idx2 = 0;
                x1 = root.rankpair(pos1, ROOT_LEFT); 
                x2 = root.rankpair(pos2, ROOT_LEFT);
                intersect_helper(x1, x2, child_idx1, child_idx2, start, end/2, intersection);
            }

        if ((root_sym1 == ROOT_RIGHT || root_sym1 == ROOT_BOTH) &&
            (root_sym2 == ROOT_RIGHT || root_sym2 == ROOT_BOTH)) {
                child_idx1 = 1;
                child_idx2 = 1;
                x1 = root.rankpair(pos1, ROOT_RIGHT);
                x2 = root.rankpair(pos2, ROOT_RIGHT);
                intersect_helper(x1, x2, child_idx1, child_idx2, end/2, end, intersection);
            }
        return intersection;
    }

    

    vector<int64_t> union_two(int64_t pos1, int64_t pos2) {
        vector<int64_t> union_set; 

        int64_t start = 0, end = alphabet.size();

        int64_t x1_left = root.rankpair(pos1, ROOT_LEFT);
        int64_t x1_right = root.rankpair(pos1, ROOT_RIGHT); 

        int64_t x2_left = root.rankpair(pos2, ROOT_LEFT);
        int64_t x2_right = root.rankpair(pos2, ROOT_RIGHT); 

        char root_sym1 = get_root_sym(pos1); 
        char root_sym2 = get_root_sym(pos2);

        if (root_sym1 == root_sym2) {
            if (root_sym1 == ROOT_BOTH || root_sym1 == ROOT_LEFT) {
                union_helper(x1_left, x2_left, 0, 0, start, end/2, union_set);
            } 
            if (root_sym1 == ROOT_BOTH || root_sym1 == ROOT_RIGHT) {
                union_helper(x1_right, x2_right, 1, 1, end/2, end, union_set);
            }
        } else {
            if (root_sym1 == ROOT_BOTH) {
                if (root_sym2 == ROOT_LEFT) {
                    union_helper(x1_left, x2_left, 0, 0, start, end/2, union_set); 
                    collect_set(x1_right, 1, end/2, end, union_set); 
                } else {
                    collect_set(x1_left, 0, 0, end/2, union_set); 
                    union_helper(x1_right, x2_right, 1, 1, end/2, end, union_set); 
                }
            } else if (root_sym1 == ROOT_LEFT) {
                if (root_sym2 == ROOT_RIGHT) {
                    collect_set(x1_left, 0, 0, end/2, union_set);
                } else {
                    union_helper(x1_left, x2_left, 0, 0, start, end/2, union_set); 
                }
                collect_set(x2_right, 1, end/2, end, union_set); 
            } else {
                collect_set(x2_left, 0, 0, end/2, union_set);
                if (root_sym2 == ROOT_LEFT) {
                    collect_set(x1_right, 1, end/2, end, union_set);
                } else {
                    union_helper(x1_right, x2_right, 1, 1, end/2, end, union_set); 
                }
            }
        }
        return union_set;
    }

    

    void union_range_helper(int64_t child_idx, int64_t l, int64_t r, int64_t start, int64_t end, vector<int64_t>& union_set) const {
        if (l == r) {
            collect_set(l, child_idx, start, end, union_set);
            return;
        }
        if (child_idx >= children.size() || !children[child_idx]) {
            union_set.insert(union_set.end(), alphabet.begin()+start, alphabet.begin()+end);
            return;
        }

        const auto& interval = *child_intervals[child_idx];
        int64_t left = interval.first;
        int64_t right = interval.second;

        pair<vector<int64_t>, vector<int64_t>> syms = extract_child_syms(child_idx, l, r);
        vector<int64_t> left_child_syms = syms.first;
        vector<int64_t> right_child_syms = syms.second;

        if (left_child_syms.size() > 0) {
            int64_t l_left = children[child_idx]->rankpair(left_child_syms.front(), CHILD_LEFT);
            int64_t r_left = children[child_idx]->rankpair(left_child_syms.back(), CHILD_LEFT);
            union_range_helper(get_left_child_idx(child_idx), l_left, r_left, start, (left+right)/2, union_set);
        }

        if (right_child_syms.size() > 0) {
            int64_t l_right = children[child_idx]->rankpair(right_child_syms.front(), CHILD_RIGHT);
            int64_t r_right = children[child_idx]->rankpair(right_child_syms.back(), CHILD_RIGHT);
            union_range_helper(get_right_child_idx(child_idx), l_right, r_right, (left+right)/2, end, union_set);
        }
    }

    vector<int64_t> union_range(int64_t left, int64_t right) { 
        if (left == right) {
            return extract_set(left);
        }

        if (left > right) {
            throw std::out_of_range("");
        }

        vector<int64_t> union_set; 

        int64_t start=0, end=alphabet.size(); 
        
        pair<vector<int64_t>, vector<int64_t>> syms = extract_syms(left, right);

        vector<int64_t> left_syms = syms.first;
        vector<int64_t> right_syms = syms.second;
        if (left_syms.size() > 0) {
            int64_t l_left = root.rankpair(left_syms.front(), ROOT_LEFT);
            int64_t r_left = root.rankpair(left_syms.back(), ROOT_LEFT);
            union_range_helper(0, l_left, r_left, start, end/2, union_set);
        }

        if (right_syms.size() > 0) {
            int64_t l_right = root.rankpair(right_syms.front(), ROOT_RIGHT);
            int64_t r_right = root.rankpair(right_syms.back(), ROOT_RIGHT);
            union_range_helper(1, l_right, r_right, end/2, end, union_set);
        }

        return union_set;
    }


    size_t size_in_bytes() const{
        size_t sz = 0; 
        sz += sizeof(int64_t)*alphabet.capacity();
        sz += sizeof(int64_t)*char_to_idx.capacity();
        sz += root.size_in_bytes();
        sz += children.capacity() * sizeof(optional<base3_rank_t>);
        for (const auto&c : children) {
            if (c.has_value()) {
                sz += c->size_in_bytes();
            }
        }
        sz += child_intervals.capacity() * sizeof(optional<pair<int64_t, int64_t>>);
        return sz;
    }

    int64_t serialize(ostream& os) const{
        return 0; // TODO
    }

    void load(istream& is){
        return; // TODO
    }


};
