#pragma once

#include "board.h"
#include "defs.h"

namespace Perft {
void Driver(Board &board, int depth);
void Test(Board &board, int depth);
} // namespace Perft
