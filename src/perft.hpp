#pragma once

#include "board.hpp"
#include "defs.hpp"

namespace Perft {
void Driver(Board &board, int depth);
void Test(Board &board, int depth);
} // namespace Perft
