#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <utility>
#include <bitset>
#include <vector>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>


using _Pos_Type = std::pair <int,int>;
using _Pos_Hash = struct {
    size_t operator () (const _Pos_Type &__pos) const noexcept {
        return (__pos.first << 16) | __pos.second;
    }
};

using _Pos_List = std::vector <_Pos_Type>;
using _Pos_Set  = std::unordered_set <_Pos_Type,_Pos_Hash>;

struct _Node_Type {
    int  x, y;
    bool flag;
};


using _Node_Hash = struct {
    size_t operator () (const _Node_Type &__edge) const noexcept {
        return (__edge.x << 16) | __edge.y;
    }
};
using _Node_EQ    = struct {
    bool operator () (const _Node_Type &__a,const _Node_Type &__b) const noexcept {
        return __a.x == __b.x && __a.y == __b.y && __a.flag == __b.flag;
    }
};

using _Node_Set = std::unordered_set <_Node_Type,_Node_Hash,_Node_EQ>;

using _Node_Map = std::unordered_map <_Node_Type,_Node_Set,_Node_Hash,_Node_EQ>;


struct state {
  public:
    inline static const uint8_t NPOS        = 255;
    inline static const uint8_t MINE_BIT    = 1u << 7;
    inline static const uint8_t SAFE_BIT    = 1u << 6;

    inline static const uint8_t GUESS_MINE  = 1u << 5;
    inline static const uint8_t GUESS_SAFE  = 1u << 4;

    inline static const uint8_t UNKNOWN_BIT = 1u << 0;

  private:
    uint8_t mine = 0; /* Count of mines. (in range 0 ~ 8)*/
    uint8_t flag = 0; /* Extra control flag.             */

  public:
    /* Marking a position that is safe and visited. */
    void set_visited(uint8_t __cnt) noexcept {
        mine = __cnt;
        flag = 0;
    }
    /* Marking a position that is unknown. */
    void set_unknown() noexcept {
        mine = NPOS;
        flag = UNKNOWN_BIT;
    }
    /* Marking a position that is definitely a mine. */
    void set_mine() noexcept {
        mine = NPOS;
        flag = MINE_BIT;
    }
    /* Marking a position that is definitely safe. */
    void set_safe() noexcept {
        mine = NPOS;
        flag = SAFE_BIT;
    }

    /* Reset all guessed to safe. */
    void reset_guess() noexcept {
        if (flag & (GUESS_MINE | GUESS_SAFE)) {
            mine = NPOS;
            flag = UNKNOWN_BIT;
        }
    }
    /* Set the guessing bit. */
    void set_guess_mine() noexcept {
        mine = NPOS;
        flag = GUESS_MINE;
    }
    /* Set the guessing bit. */
    void set_guess_safe() noexcept {
        mine = NPOS;
        flag = GUESS_SAFE;
    }

    bool is_visited() const noexcept {
        return mine != NPOS;
    }
    bool is_unknown() const noexcept {
        return flag & UNKNOWN_BIT;
    }
    bool is_definitely_mine() const noexcept {
        return flag & MINE_BIT;
    }
    bool is_definitely_safe() const noexcept {
        return flag & SAFE_BIT;
    }
    uint8_t get_mine_count() const noexcept {
        return mine;
    }
    bool is_guessed_mine() const noexcept {
        return flag & GUESS_MINE;
    }
    bool is_guessed_safe() const noexcept {
        return flag & GUESS_SAFE;
    }
};


extern int rows;     // The count of rows of the game map
extern int columns;  // The count of columns of the game map

inline static _Pos_List work_list = {};
inline static constexpr _Pos_Type   kNOTFOUND   = {0,0};
inline static constexpr size_t      kMAPSIZE    = 64;
inline state map[kMAPSIZE][kMAPSIZE] = {};


void _Debug() {
    std::cerr<< "----------- Current:" << std::endl;
    for(int i = 1 ; i <= rows ; ++i) {
        for(int j = 1 ; j <= columns ; ++j) {
            if (map[i][j].is_visited()) {
                std::cerr<< (int)map[i][j].get_mine_count();
            } else if (map[i][j].is_unknown()) {
                std::cerr<< '?';
            } else if (map[i][j].is_definitely_mine()) {
                std::cerr<< 'X';
            } else if (map[i][j].is_definitely_safe()) {
                std::cerr<< 'O';
            } else if (map[i][j].is_guessed_mine()) {
                std::cerr<< 'x';
            } else if (map[i][j].is_guessed_safe()) {
                std::cerr<< 'o';
            }
        }
        std::cerr<< std::endl;
    }
}


void Execute(int row, int column);

void InitGame() {
    int first_row, first_column;
    std::cin >> first_row >> first_column;
    Execute(first_row, first_column);
}

void ReadMap() {
    work_list.clear();
    for (int i = 1 ; i <= rows ; ++i) {
        std::string __buf;
        std::cin >> __buf;
        for (int j = 1 ; j <= columns ; ++j) {
            auto __cur = __buf[j - 1]; /* Current char. */
            if (std::isdigit(__cur)) {
                map[i][j].set_visited(__cur - '0');
                work_list.emplace_back(i,j);
            } else {
                map[i][j].set_unknown();
            }
        }
    }
}

template <class ..._Func>
uint8_t count_if(int x,int y,_Func &&...__cond) {
    uint8_t __cnt = 0;
    for(int i = x - 1 ; i <= x + 1 ; ++i) {
        for(int j = y - 1 ; j <= y + 1 ; ++j) {
            __cnt += (__cond(i,j) || ...);
        }
    }
    return __cnt;
}

template <class _Func>
void update(int x,int y,_Func &&__work) {
    for(int i = x - 1 ; i <= x + 1 ; ++i) {
        for(int j = y - 1 ; j <= y + 1 ; ++j) {
            __work(i,j);
        }
    }
}

/**
 * @brief Reset the guessing state.
 * Use it before any guessing.
*/
void init_guessing() {
    for(int i = 1 ; i <= rows ; ++i) {
        for(int j = 1 ; j <= columns ; ++j) {
            map[i][j].reset_guess();
        }
    }
}

/**
 * @brief Collect all unknown adjacent to visited.
 * @return The list of required data (no duplicate).
 */
_Pos_List collect_adjacent_unknown() {
    _Pos_Set __set = {};
    auto &&__collect_unknown = [&](int x,int y) -> void {
        if (map[x][y].is_unknown()) __set.emplace(x,y);
    };
    for(int i = 1 ; i <= rows ; ++i) {
        for(int j = 1 ; j <= columns ; ++j) {
            if (map[i][j].is_visited())
                update(i,j,__collect_unknown);
        }
    }
    return _Pos_List {__set.begin(),__set.end()};
}


bool is_in_range(int x,int y) {
    return x >= 1 && x <= rows && y >= 1 && y <= columns;
}
bool is_mine(int x,int y) {
    return map[x][y].is_definitely_mine();
}
bool is_unknown(int x,int y) {
    return map[x][y].is_unknown();
}
void mark_mine(int x,int y) {
    if (map[x][y].is_unknown()) map[x][y].set_mine();
}
void mark_safe(int x,int y) {
    if (map[x][y].is_unknown()) map[x][y].set_safe();
}
bool may_be_mine(int x,int y) {
    return map[x][y].is_guessed_mine();
}
bool may_be_safe(int x,int y) {
    return map[x][y].is_guessed_safe();
}
void push_list(int x,int y) {
    update(x,y,[](int x,int y) {
        if (is_in_range(x,y)) {
            work_list.emplace_back(x,y);
        }
    });
}
void mark_possible_mine(int x,int y) {
    if (map[x][y].is_unknown()) {
        map[x][y].set_guess_mine();
        push_list(x,y);
    }
}
void mark_possible_safe(int x,int y) {
    if (map[x][y].is_unknown()) {
        map[x][y].set_guess_safe();
        push_list(x,y);
    }
}

/* Tries to update a visited node state. */
bool try_update_round(int x,int y) {
    /* Unknown blank counts. */
    auto __unknowns = count_if(x,y,is_unknown);
    /* Detected mine counts */
    auto __detected = count_if(x,y,is_mine);
    /* Indicated mine counts */
    auto __indicate = map[x][y].get_mine_count();

    if (!__unknowns) {
        return false;
    } else if (__indicate == __detected + __unknowns) {
        update(x,y,mark_mine);
        return true;
    } else if (__indicate == __detected) {
        update(x,y,mark_safe);
        return true;
    } else {
        return false;
    }
}


/**
 * @brief Tries to find a safe node.
 * Init the worklist before calling this function.
 * @return The position of a safe node.
 * If not found, return kNOTFOUND
*/
_Pos_Type take_safe() {
    while (!work_list.empty()) {
        auto [x , y] = work_list.back();
        work_list.pop_back();

        /* Find one answer. */
        if (map[x][y].is_definitely_safe()) return {x,y};

        /* Only visited node can be updating. */
        if (!map[x][y].is_visited()) continue;

        /* If success, add to worklist. */
        if (try_update_round(x,y)) push_list(x,y);
    }
    return kNOTFOUND;
}


/**
 * @brief Tries to find contraction.
 * Init the worklist before calling this function.
 * @return True iff a contraction is found.
*/
bool find_contratiction() {
    while(!work_list.empty()) {
        auto [x,y] = work_list.back();
        work_list.pop_back();

        if (!map[x][y].is_visited()) continue;

        /* Not marked. */
        auto __unknowns = count_if(x,y,is_unknown);
        /* Indication of mine count. */
        auto __indicate = map[x][y].get_mine_count();
        /* Definitely mine. */
        auto __possible = count_if(x,y,is_mine,may_be_mine);
        if (!__unknowns) {
            if (__indicate != __possible) return true;
            else continue;
        } else if (__indicate == __possible + __unknowns) {
            update(x,y,mark_possible_mine);
        } else if (__indicate == __possible) {
            update(x,y,mark_possible_safe);
        } else if (__possible > __indicate) {
            return true;
        }
    }
    return false;
}


_Pos_Type guess_mine(const _Pos_List &__list) {
    work_list.clear();
    for(auto [x , y] : __list) {
        init_guessing();

        map[x][y].set_guess_mine();
        work_list.clear();
        push_list(x,y);

        /* pos(x,y) cannot be a mine! */
        if (find_contratiction()) return {x,y};
    }
    return kNOTFOUND;
}


_Pos_Type guess_safe(const _Pos_List &__list) {
    _Pos_List __updated = {};
    for(auto [x , y] : __list) {
        init_guessing();

        map[x][y].set_guess_safe();
        work_list.clear();
        push_list(x,y);

        /* pos(x,y) must be a mine! */
        if (find_contratiction()) {
            map[x][y].set_mine();
            __updated.emplace_back(x,y);
        }
    }

    /* No guessing state is needed. */
    init_guessing();

    if (__updated.empty()) {
        work_list.resize(1);
        return kNOTFOUND;
    } else {
        work_list = std::move(__updated);
        return take_safe();
    }
}


inline static _Node_Map graph = {};


void set_contradiction(_Node_Type __lhs,_Node_Type __rhs) {
    __rhs.flag = !__rhs.flag;
    graph[__lhs].insert(__rhs);

    __rhs.flag = !__rhs.flag;
    __lhs.flag = !__lhs.flag;
    graph[__rhs].insert(__lhs);
}


_Node_Set dfs(_Node_Type __cur) {
    _Node_Set visited = {};
    std::vector <_Node_Type> stack = {};
    auto &__ref = graph.at(__cur);
    stack.push_back(__cur);

    while (!stack.empty()) {
        auto __top = stack.back();
        stack.pop_back();
        if (!visited.emplace(__top).second) continue;

        auto __iter = graph.find(__top);
        if (__iter == graph.end()) continue;

        for(auto __node : __iter->second) stack.push_back(__node);
    }

    return visited;
}


_Pos_Type build_graph(_Pos_List &__list) {
    for(auto [x , y] : __list) {
        _Node_Type __cur = {x,y};
        __cur.flag  = true;
        auto __mine = graph.find(__cur);
        if (__mine == graph.end()) continue;
        __cur.flag  = false;

        __mine->second = dfs(__mine->first);

        /* Add to the contra list! */
        if (__mine->second.count(__cur)) return {x,y};
    }
    return kNOTFOUND;
}


_Pos_Type guess_double(_Pos_List &__list) {
    graph.clear();
    work_list.clear();
    for(size_t i = 0 ; i < __list.size() ; ++i) {
        for(size_t j = i + 1 ; j < __list.size() ; ++j) {
            auto [x1 , y1] = __list[i];
            auto [x2 , y2] = __list[j];

            _Node_Type __lhs = {x1,y1};
            _Node_Type __rhs = {x2,y2};

            work_list.clear();
            map[x1][y1].set_guess_mine();
            map[x2][y2].set_guess_mine();
            push_list(x1,y1);
            push_list(x2,y2);
            __lhs.flag = true;
            __rhs.flag = true;
            if (find_contratiction()) set_contradiction(__lhs,__rhs);
            init_guessing();

            work_list.clear();
            map[x1][y1].set_guess_mine();
            map[x2][y2].set_guess_safe();
            push_list(x1,y1);
            push_list(x2,y2);
            __lhs.flag = true;
            __rhs.flag = false;
            if (find_contratiction()) set_contradiction(__lhs,__rhs);
            init_guessing();

            work_list.clear();
            map[x1][y1].set_guess_safe();
            map[x2][y2].set_guess_mine();
            push_list(x1,y1);
            push_list(x2,y2);
            __lhs.flag = false;
            __rhs.flag = true;
            if (find_contratiction()) set_contradiction(__lhs,__rhs);
            init_guessing();

            work_list.clear();
            map[x1][y1].set_guess_safe();
            map[x2][y2].set_guess_safe();
            push_list(x1,y1);
            push_list(x2,y2);
            __lhs.flag = false;
            __rhs.flag = false;
            if (find_contratiction()) set_contradiction(__lhs,__rhs);

            init_guessing();
        }
    }

    if (auto [x , y] = build_graph(__list); x != 0) return {x,y};
    return kNOTFOUND;
}


_Pos_Type guessing() {
    std::cerr << "Guess single!\n";
    do {
        _Pos_List __list = collect_adjacent_unknown();
        if (auto [x , y] = guess_mine(__list); x != 0) return {x,y};
        if (auto [x , y] = guess_safe(__list); x != 0) return {x,y};
    } while(work_list.empty());
    init_guessing();

    /* Single guess failed! */
    std::cerr << "Guess double!\n";
    _Pos_List __list = collect_adjacent_unknown();
    if (auto [x , y] = guess_double(__list); x != 0) return {x,y};
    bool     __flag = false;
    _Pos_Type __ans = {0,0};
    for(int i = 1 ; i <= rows ; ++i) {
        for(int j = 1 ; j <= columns ; ++j) {
            if (map[i][j].is_unknown()) {
                if (__flag) return kNOTFOUND;
                __flag = true;
                __ans  = {i,j};
            }
        }
    }
    return __ans;
}

inline static constexpr double global_average = 0.2;

double calc_prob(int x,int y) {
    if (!map[x][y].is_visited()) return global_average;
    auto __unknowns = count_if(x,y,is_unknown);
    auto __indicate = map[x][y].get_mine_count();
    auto __detected = count_if(x,y,is_mine);
    if (__unknowns == 0) return 0.0;
    return (__indicate - __detected) / static_cast <double> (__unknowns);
}

double __prob[kMAPSIZE][kMAPSIZE] = {};

_Pos_Type take_random() {
    // double    __min = 1.0;
    // _Pos_Type __ans = {1,1};
    std::cerr << "Take risk!\n";

    double __tmp = 0.0;
    for(int i = 1 ; i <= rows ; ++i) {
        for(int j = 1 ; j <= columns ; ++j) {
            __prob[i][j] = calc_prob(i,j);
        }
    }
    auto &&__amort_prob = [&](int x,int y) -> void {
        if (is_in_range(x,y) && __prob[x][y] != global_average ) {
            __tmp = std::max(__tmp,__prob[x][y]);
        }
    };
    std::vector <std::pair <double,_Pos_Type>> __list = {};
    for(int i = 1 ; i <= rows ; ++i) {
        for(int j = 1 ; j <= columns ; ++j) {
            if (map[i][j].is_unknown()) {
                __tmp = global_average;
                update(i,j,__amort_prob);
                __list.emplace_back(__tmp,std::make_pair(i,j));
            }
        }
    }
    std::sort (__list.begin(),__list.end(),[](const auto &__a,const auto &__b) {
        return __a.first < __b.first;
    });
    return __list[0].second;
}

void Decide() {
    _Debug();
    if (auto [x , y] = take_safe(); x != 0) return Execute(x - 1,y - 1);
    // if (auto [x , y] = guessing() ; x != 0) return Execute(x - 1,y - 1);
    auto [x , y] = take_random();
    return Execute(x - 1,y - 1);
}

#endif