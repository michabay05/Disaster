#pragma once

#include "board.hpp"
#include "defs.hpp"

namespace Perft {
void driver(Board& board, const int depth);
void test(Board& board, const int depth);
} // namespace Perft
