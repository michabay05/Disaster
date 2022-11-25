#pragma once

#include "board.hpp"
#include "defs.hpp"

namespace Evaluate {
extern const int materialScore[12];
int EvalPosition(const Board &board);

} // namespace Evaluate
