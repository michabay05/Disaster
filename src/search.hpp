#pragma once

#include "defs.hpp"
#include "board.hpp"

namespace Search {

void position(Board &board, int depth);
int negamax(Board *board, int alpha, int beta, int depth);
int quiescence(Board *board, int alpha, int beta);
int scoreMoves(Board &board, int move);
}
