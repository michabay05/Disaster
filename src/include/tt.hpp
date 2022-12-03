#pragma once

#include "board.hpp"
#include "defs.hpp"

namespace TT {

extern const int NO_ENTRY;

void clearTTtable();

enum TTFlags { F_EXACT, F_ALPHA, F_BETA };

struct TTEntry {
    uint64_t hashKey = 0ULL;
    uint64_t hashLock = 0ULL;
    int score = 0;
    int depth = 0;
    int flag = 0;
};

int readEntry(const Board& board, const int alpha, const int beta, const int depth);
void writeEntry(const Board& board, const int depth, int score, const int flag);
} // namespace TT
