#pragma once

#include "board.hpp"
#include "defs.hpp"

namespace TT::Eval {

struct EvalEntry {
    uint64_t key = 0ULL;
    int eval = 0;
};

void clearEvalTable();
int readEntry(const Board& board);
void writeEntry(const Board& board, int eval);
} // namespace TT::Eval
