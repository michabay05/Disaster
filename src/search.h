#pragma once

#include "defs.h"
#include "board.h"

namespace Search {
#define INFINITY 50'000
// Upper and lower bound
#define MATE_VALUE 49'000
#define MATE_SCORE 48'000

#define MAX_PLY 64
#define FULL_DEPTH_MOVES 4
#define REDUCTION_LIMIT 3

void position(Board &board, int depth);
int negamax(Board &board, int alpha, int beta, int depth);
int quiescene(Board &board, int alpha, int beta);
}
