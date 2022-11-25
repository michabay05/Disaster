#pragma once

#include "board.hpp"
#include "defs.hpp"
#include "move.hpp"

#include <array>

namespace Search {

#define INF 50'000

// Score layout (Black advantage -> 0 -> White advantage)
// -INFINITY < -MATE_VALUE < -MATE_SCORE < NORMAL(non - mating) score < MATE_SCORE < MATE_VALUE < INFINITY

// Upper and lower bound
#define MATE_VALUE 49'000
#define MATE_SCORE 48'000

#define MAX_PLY 64
#define FULL_DEPTH_MOVES 4
#define REDUCTION_LIMIT 3

extern int ply, nodes;

void position(Board &board, const int depth);
int negamax(Board *board, const int alpha, const int beta, const int depth);
int quiescence(Board *board, const int alpha, const int beta);
int scoreMoves(const Board &board, const int move);
void printMoveScores(const Move::MoveList &moveList, const Board &board);
void sortMoves(Move::MoveList &moveList, const Board &board);
void clearSearchTable();
void enablePVScoring(Move::MoveList &moveList);

} // namespace Search
