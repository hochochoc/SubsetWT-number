#pragma once 

#include <vector>
#include <utility>
#include <iostream>
#include <optional>
#include <stack>
#include <unordered_set>
#include <unordered_map>
#include <fstream>

#include <chrono>
#include <cassert>

using namespace std;

template<typename base4_rank_t, typename perm_rank_t>
class SubsetWT{

public:

    static const int64_t ROOT_NONE = 0; // '00'
    static const int64_t ROOT_RIGHT = 1; // '01'
    static const int64_t ROOT_LEFT = 2; // '10'
    static const int64_t ROOT_BOTH = 3; // '11'

    static const int64_t CHILD_RIGHT = 1; // '01'
    static const int64_t CHILD_LEFT = 2; // '10'
    static const int64_t CHILD_BOTH = 3; // '11'
    static const int64_t CHILD_ALL = 0; // '?'

    vector<int64_t> alphabet;
    vector<int64_t> char_to_idx; // Vector of length 256 mapping characters to their indices in the alphabet
    
    base4_rank_t root;

    // children[0] and children[1] are the left and right children of the root
    // The children of children[i] are at children[2*i+2] and children[2*i + 3]
    vector<optional<base4_rank_t>> children;

    // Alphabet intervals of child nodes
    vector<optional<pair<int64_t, int64_t>>> child_intervals;

    perm_rank_t sp;
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
        cout << "Initiated alphabet" << endl;
    }

    bool leaf_next(int64_t start, int64_t end) {
        return (end-start <= 1);
    }

    struct Task {
        int64_t child_idx;
        int64_t start, end;
        vector<int64_t> sets_in_this_child;
        bool post_process; 

        vector<int64_t> both_index_in_left;
        vector<int64_t> both_index_in_right;
        vector<int64_t> index_sets_to_both;

        Task(int64_t cidx, int64_t s, int64_t e, vector<int64_t> &&sets, bool post = false) 
            : child_idx(cidx), start(s), end(e), sets_in_this_child(std::move(sets)), post_process(post) {}
    };

    // Helper function used in the constructor
    // Alphabet must be initialized before calling
    // [start, end) is a half-open interval
    vector<char> init_children_recursion(int64_t child_idx, int64_t start, int64_t end, const vector<vector<int64_t>>& sets, vector<int64_t>&& sets_in_this_child){
        stack<Task> task_stack;

        unordered_map<int64_t, vector<char>> split_seqs;

        task_stack.emplace(child_idx, start, end, std::move(sets_in_this_child));

        while (!task_stack.empty()) 
        {
            Task task = std::move(task_stack.top());
            task_stack.pop();

            int64_t child_idx = task.child_idx;
            int64_t start = task.start;
            int64_t end = task.end;
            int64_t mid = (task.start+task.end)/2;

            int64_t left_idx = get_left_child_idx(child_idx);
            int64_t right_idx = get_right_child_idx(child_idx);

            if (task.post_process) {
                bool left_leaf = leaf_next(start, mid);
                bool right_leaf = leaf_next(mid, end);

                auto& left_seq = split_seqs[left_idx];
                auto& right_seq = split_seqs[right_idx];
                auto& this_seq = split_seqs[child_idx];

                for (int isx = task.index_sets_to_both.size() - 1; isx >= 0; --isx) {
                    int64_t parent_idx = task.index_sets_to_both[isx];
                    int64_t left_child_idx = task.both_index_in_left[isx];
                    int64_t right_child_idx = task.both_index_in_right[isx];
                   
                    if ((left_leaf || left_seq[left_child_idx] == CHILD_ALL) && (right_leaf || right_seq[right_child_idx] == CHILD_ALL)) {
                        this_seq[parent_idx] = CHILD_ALL;
                        if (!left_leaf) {
                            left_seq.erase(left_seq.begin() + left_child_idx);
                        }
                        if (!right_leaf) {
                            right_seq.erase(right_seq.begin() + right_child_idx);
                        }
                    }
                }

                if (!left_leaf) {
                    children[left_idx] = base4_rank_t(left_seq);
                }

                if (!right_leaf) {
                    children[right_idx] = base4_rank_t(right_seq);
                }
                
                continue;
            }

            // Pre-process

            if (leaf_next(start, end)) continue;
            
            child_intervals[child_idx] = {start, end};
            int64_t middle_char = alphabet[(start+end)/2];
            // [start, midpoint) go left, [midpoint, end) go right

            // Set indexes that go to left and to right
            vector<int64_t> sets_to_left, sets_to_right;

            vector<int64_t> both_index_in_left, both_index_in_right, index_sets_to_both;
            vector<char> split_seq; // 0 = left, 1 = right, 2 = both

            // Split the alphabet
            for(size_t idx = 0; idx < task.sets_in_this_child.size(); idx++) {
                bool has_left = false;
                bool has_right = false;
                int64_t i = task.sets_in_this_child[idx];
                for(int64_t c : sets[i]){
                    if(c >= alphabet[start] && c < middle_char){
                        has_left = true;
                    } else if(c >= middle_char && (end == alphabet.size() || c < alphabet[end])){
                        has_right = true;
                    }
                    if (has_left && has_right) {
                        break;
                    }
                }
                if(has_left && !has_right) split_seq.push_back(CHILD_LEFT);
                else if(!has_left && has_right) split_seq.push_back(CHILD_RIGHT);
                else split_seq.push_back(CHILD_BOTH);

                if (has_left && has_right) {
                    both_index_in_left.push_back(sets_to_left.size()); // index of Both in left child
                    both_index_in_right.push_back(sets_to_right.size()); // index of Both in right child
                    index_sets_to_both.push_back(idx); // index of Both in current node, values of index_sets_to_both match index of split_seq
                } 
                if(has_left) sets_to_left.push_back(i);
                if(has_right) sets_to_right.push_back(i);
            }

            Task post_task(child_idx, start, end, {});
            post_task.index_sets_to_both = std::move(index_sets_to_both);
            post_task.both_index_in_left = std::move(both_index_in_left);
            post_task.both_index_in_right = std::move(both_index_in_right);
            post_task.post_process = true; 
            split_seqs[child_idx] = std::move(split_seq);
            task_stack.push(std::move(post_task));

            if (left_idx < children.size()) {
                task_stack.emplace(left_idx, start, mid, std::move(sets_to_left));
            }
            if (right_idx < children.size()) {
                task_stack.emplace(right_idx, mid, end, std::move(sets_to_right));
            }
            
            continue;
        }
        return split_seqs[child_idx];
    }

    // Helper function used in the constructor
    // Alphabet must be initialized before calling
    void init_tree(const vector<vector<int64_t>>& sets){
        int64_t sigma = alphabet.size();
        int64_t middle_char = alphabet[sigma/2];

        // Initialize the root
        vector<char> root_split_seq;
        vector<int64_t> sets_to_left_child, sets_to_right_child;
        vector<int64_t> both_index_in_left, both_index_in_right, index_sets_to_both;

        for(size_t i = 0; i < sets.size(); i++) {
            bool has_left = false;
            bool has_right = false;
            // TODO: increase time usage here 
            // color set is sorted 
            for(int64_t c : sets[i]){
                if (has_left && has_right) {
                    break;
                }
                if(c < middle_char) has_left = true;
                else has_right = true;
            }

            assert (has_left || has_right); // not allow empty subsets

            if(has_left && !has_right) root_split_seq.push_back(ROOT_LEFT);
            else if(!has_left && has_right) root_split_seq.push_back(ROOT_RIGHT);
            else if(has_left && has_right) root_split_seq.push_back(ROOT_BOTH);

            if (has_left && has_right) {
                both_index_in_left.push_back(sets_to_left_child.size()); // index of Both in left child
                both_index_in_right.push_back(sets_to_right_child.size()); // index of Both in right child
                index_sets_to_both.push_back(i); // index of Both in current node, values of index_sets_to_both match index of split_seq
            } 

            if(has_left) sets_to_left_child.push_back(i);
            if(has_right) sets_to_right_child.push_back(i);
        }

        // Initialize space for the children
        child_intervals.resize(2*sigma);
        children.resize(2*sigma);

        int64_t mid = sigma/2;

        // Initialize the children
        vector<char> left_symbols = init_children_recursion(0, 0, mid, sets, std::move(sets_to_left_child)); // Left child of root
        vector<char> right_symbols = init_children_recursion(1, mid, sigma, sets, std::move(sets_to_right_child)); // Right child of root


        // Construct the base-4 rank support
        for (size_t isx = 0; isx < index_sets_to_both.size(); isx ++) {
            // leaf or ALL in next child 
            if (((leaf_next(0, mid) || left_symbols[both_index_in_left[isx]]== CHILD_ALL) && (leaf_next(mid, sigma) || right_symbols[both_index_in_right[isx]] == CHILD_ALL))) {
                root_split_seq[index_sets_to_both[isx]] = ROOT_NONE;
                if (!leaf_next(0, mid)) {
                    left_symbols.erase(left_symbols.begin() + both_index_in_left[isx]); // remove All symbol from left child's symbol list 
                }

                if (!leaf_next(mid, sigma)) {
                    right_symbols.erase(right_symbols.begin() + both_index_in_right[isx]); // remove All symbol from left child's symbol list 
                }
            }
        }

        if (!leaf_next(0, mid)) {
            children[0] = base4_rank_t(left_symbols); // encode base-4 sequence 
        }

        if (!leaf_next(mid, sigma)) {
            children[1] = base4_rank_t(right_symbols);
        }

        // what base4_rank_t does?
        root = base4_rank_t(root_split_seq);
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

    struct SingleFrame {
        int64_t pos; 
        int64_t child_idx; 
        int64_t start;
        int64_t end;
    };

    void collect_set_stack(int64_t pos, int64_t child_idx, int64_t start, int64_t end, vector<int64_t>& result_set) const {
        
        std::stack<SingleFrame> stk; 
        stk.push({pos, child_idx, start, end});
        // Traverse deeper into the children 
        // how big of children
        while(!stk.empty()) {
            SingleFrame f = stk.top(); stk.pop(); 

            if (f.child_idx >= children.size() || !children[f.child_idx] || isChildrenAll(f.pos, f.child_idx)) {
                result_set.insert(result_set.end(), alphabet.begin() + f.start, alphabet.begin() + f.end);
                continue;
            }
            const auto& interval = *child_intervals[f.child_idx];
            int64_t left = interval.first;
            int64_t right = interval.second; // get child range 
            int64_t mid = (left+right)/2;

            const auto& child_rank = *children[f.child_idx];

            int64_t left_index = child_rank.rankpair(f.pos, CHILD_LEFT);
            int64_t right_index = child_rank.rankpair(f.pos, CHILD_RIGHT);
            int64_t prev_left_index = child_rank.rankpair(f.pos-1, CHILD_LEFT);
            int64_t prev_right_index = child_rank.rankpair(f.pos-1, CHILD_RIGHT);

            if (right_index > prev_right_index) {
                stk.push({right_index, get_right_child_idx(f.child_idx), mid, f.end});
            }

            if (left_index > prev_left_index) {
                stk.push({left_index, get_left_child_idx(f.child_idx), f.start, mid});
            }            
        }
    }

    struct PairFrame {
        int64_t x1, x2; 
        int64_t child_idx; 
        int64_t start, end; 
    };

    void intersect_helper(int64_t x1, int64_t x2, int64_t child_idx, int64_t start, int64_t end, vector<int64_t>& intersection) {

        std::stack<PairFrame> stk; 
        stk.push({x1, x2, child_idx, start, end}); 

        while (!stk.empty()) {
            auto [x1, x2, child_idx, start, end] = stk.top(); stk.pop(); 
            if (child_idx >= children.size() || !children[child_idx] || (isChildrenAll(x1, child_idx) && isChildrenAll(x2, child_idx))) {
                intersection.insert(intersection.end(), alphabet.begin()+start, alphabet.begin()+end);
                continue;
            }

            if (isChildrenAll(x1, child_idx)) {
                collect_set_stack(x2, child_idx, start, end, intersection);
                continue;
            }

            if (isChildrenAll(x2, child_idx)) {
                collect_set_stack(x1, child_idx, start, end, intersection);
                continue;
            }
    
            const auto& interval = *child_intervals[child_idx];
            int64_t left = interval.first;
            int64_t right = interval.second;
            int64_t mid = (left+right)/2;

            const auto& child_rank = *children[child_idx];

            int64_t x1_left_index = child_rank.rankpair(x1, CHILD_LEFT);
            int64_t x1_right_index = child_rank.rankpair(x1, CHILD_RIGHT);
            
            int64_t x2_left_index = child_rank.rankpair(x2, CHILD_LEFT);
            int64_t x2_right_index = child_rank.rankpair(x2, CHILD_RIGHT);

            int64_t prev_x1_right_index = child_rank.rankpair(x1-1, CHILD_RIGHT);
            int64_t prev_x2_right_index = child_rank.rankpair(x2-1, CHILD_RIGHT); 

            int64_t prev_x1_left_index = child_rank.rankpair(x1-1, CHILD_LEFT);
            int64_t prev_x2_left_index = child_rank.rankpair(x2-1, CHILD_LEFT);

            if (x1_right_index > prev_x1_right_index && x2_right_index > prev_x2_right_index) {
                stk.push({x1_right_index, x2_right_index, get_right_child_idx(child_idx), mid, end});
            }

            if (x1_left_index > prev_x1_left_index && x2_left_index > prev_x2_left_index) {
                stk.push({x1_left_index, x2_left_index, get_left_child_idx(child_idx), start, mid});
            }
        }
    }

    void union_range_helper(int64_t child_idx, int64_t l, int64_t r, int64_t start, int64_t end, vector<int64_t>& union_set) const {
        std::stack<PairFrame> stk; 
        stk.push({l, r, child_idx, start, end}); 

        while (!stk.empty()) {

            auto [l, r, child_idx, start, end] = stk.top(); stk.pop(); 
            if (child_idx >= children.size() || !children[child_idx] || (children[child_idx]->rank(r, CHILD_ALL) > children[child_idx]->rank(l-1, CHILD_ALL))) {
                union_set.insert(union_set.end(), alphabet.begin()+start, alphabet.begin()+end);
                continue;
            }

            if (l == r) {
                collect_set_stack(l, child_idx, start, end, union_set);
                continue;
            }

            int64_t l_left = children[child_idx]->rankpair(l-1, CHILD_LEFT) + 1;
            int64_t r_left = children[child_idx]->rankpair(r, CHILD_LEFT);

            int64_t l_right = children[child_idx]->rankpair(l-1, CHILD_RIGHT) + 1;
            int64_t r_right = children[child_idx]->rankpair(r, CHILD_RIGHT);

            const auto& interval = *child_intervals[child_idx];
            int64_t left = interval.first;
            int64_t right = interval.second;

            int64_t mid = (left+right)/2;

            if (l_right <= r_right) {
                stk.push({l_right, r_right, get_right_child_idx(child_idx), mid, end});
            }
            
            if (l_left <= r_left) {
                stk.push({l_left, r_left, get_left_child_idx(child_idx), start, mid});
            }
        }
    }


public:

    SubsetWT(){}

    SubsetWT(const vector<vector<int64_t>>& sets, vector<int64_t> &perm, int64_t total){
        sp = perm_rank_t(perm);
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

    bool isChildrenAll(int64_t pos, int64_t child_idx) const {
        return children[child_idx]->rank(pos, CHILD_ALL) > children[child_idx]->rank(pos-1, CHILD_ALL);
    }

    bool isRootAll(int64_t pos) const {
        return root.rank(pos, ROOT_NONE) > root.rank(pos-1, ROOT_NONE);
    }

    pair<int64_t, bool> isLeft(int64_t pos) const {
        int64_t pos_in_child = root.rankpair(pos, CHILD_LEFT);
        return {pos_in_child, pos_in_child > root.rankpair(pos-1, CHILD_LEFT)};
    }

    pair<int64_t, bool> isRight(int64_t pos) const {
        int64_t pos_in_child = root.rankpair(pos, CHILD_RIGHT);
        return {pos_in_child, pos_in_child > root.rankpair(pos-1, CHILD_RIGHT)};
    }

    // return i-th color set as a vector of integers 
    vector<int64_t> extract_set(int64_t pos) const {
        vector<int64_t> result_set; 

        int64_t child_idx = 0; // start at root 
        int64_t start = 0, end = alphabet.size();
        int64_t mid = end/2;

        if (isRootAll(pos)) {
            return alphabet;
        }

        pair<int64_t, bool> leftInfo = isLeft(pos);
        if (leftInfo.second) {
            collect_set_stack(leftInfo.first, 0, 0, mid, result_set);
        }

        pair<int64_t, bool> rightInfo = isRight(pos);
        if (rightInfo.second) {
            collect_set_stack(rightInfo.first, 1, mid, end, result_set);
        }

        return result_set;
    }


    vector<int64_t> intersect(int64_t pos1, int64_t pos2) {
        if (pos1 > pos2) {
            throw std::out_of_range("right index should be larger than left index");
        }
        vector<int64_t> intersection;

        int64_t start = 0, end = alphabet.size();
        int64_t mid = end/2;

        if (isRootAll(pos1) && isRootAll(pos2)){
            return alphabet;
        }

        if (isRootAll(pos1)) {
            return extract_set(pos2);
        }
        if (isRootAll(pos2)) {
            return extract_set(pos1);
        }

        int64_t pos1_left_index = root.rankpair(pos1, ROOT_LEFT);
        int64_t pos1_right_index = root.rankpair(pos1, ROOT_RIGHT);

        int64_t pos2_left_index = root.rankpair(pos2, ROOT_LEFT);
        int64_t pos2_right_index = root.rankpair(pos2, ROOT_RIGHT);

        int64_t prev_post1_left_index = root.rankpair(pos1-1, ROOT_LEFT);
        int64_t prev_post2_left_index = root.rankpair(pos2-1, ROOT_LEFT);
        if (pos1_left_index > prev_post1_left_index && pos2_left_index > prev_post2_left_index) {
            intersect_helper(pos1_left_index, pos2_left_index, 0, start, mid, intersection);
        }

        int64_t prev_post1_right_index = root.rankpair(pos1-1, ROOT_RIGHT);
        int64_t prev_post2_right_index = root.rankpair(pos2-1, ROOT_RIGHT);
        if (pos1_right_index > prev_post1_right_index && pos2_right_index > prev_post2_right_index) {
            intersect_helper(pos1_right_index, pos2_right_index, 1, mid, end, intersection);
        }

        return intersection;
    }

    vector<int64_t> union_range(int64_t left, int64_t right) { 
        if (left > right) {
            throw std::out_of_range("right index should be larger than left index");
        }

        if (left == right) {
            return extract_set(left);
        }

        if (root.rank(right, ROOT_NONE) > root.rank(left-1, ROOT_NONE)) {
            return alphabet;
        }

        vector<int64_t> union_set;

        int64_t start=0, end=alphabet.size(); 
        
        int64_t l_left = root.rankpair(left-1, ROOT_LEFT) + 1;
        int64_t r_left = root.rankpair(right, ROOT_LEFT);

        int64_t l_right = root.rankpair(left-1, ROOT_RIGHT) + 1;
        int64_t r_right = root.rankpair(right, ROOT_RIGHT);

        if (l_left <= r_left) {
            union_range_helper(0, l_left, r_left, start, end/2, union_set);
        }
        
        if (l_right <= r_right) {
            union_range_helper(1, l_right, r_right, end/2, end, union_set);
        }
        
        return union_set;
    }

    vector<int64_t> flat_union(int64_t left, int64_t right) {
        unordered_set<int64_t> result_set;

        for (int64_t i = left; i <= right; ++i) {
            const vector<int64_t>& current = extract_set(i);
            result_set.insert(current.begin(), current.end());
        }

        vector<int64_t> result(result_set.begin(), result_set.end());
        std::sort(result.begin(), result.end());
        return result;
    }

    vector<int64_t> flat_intersect_two(int64_t left, int64_t right) {
        const vector<int64_t>& set1 = extract_set(left);
        const vector<int64_t>& set2 = extract_set(right);

        unordered_set<int64_t> set1_hash(set1.begin(), set1.end());
        vector<int64_t> result;

        for (int64_t val : set2) {
            if (set1_hash.count(val)) {
                result.push_back(val);
            }
        }

        std::sort(result.begin(), result.end());
        return result;
    }

    size_t size_in_bytes() const{
        size_t sz = 0; 
        sz += sizeof(int64_t)*alphabet.size();
        sz += sizeof(int64_t)*char_to_idx.size();
        sz += root.size_in_bytes();
        sz += children.size() * sizeof(optional<base4_rank_t>);
        for (const auto&c : children) {
            if (c.has_value()) {
                sz += c->size_in_bytes();
            }
        }
        sz += child_intervals.size() * sizeof(optional<pair<int64_t, int64_t>>);
        sz += sp.size_in_bytes();
        return sz;
    }

    int64_t serialize(ostream& os) const{
        return 0; // TODO
    }

    void load(istream& is){
        return; // TODO
    }
};