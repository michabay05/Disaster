#pragma once

#include "board.h"
#include "defs.h"

namespace Evaluate {
extern int materialScore[12];
int EvalPosition(Board &board);

} // namespace Evaluate
