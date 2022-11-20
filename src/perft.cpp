#include "perft.h"

#include "bitboard.h"
#include "misc.h"
#include "move.h"

namespace Perft {
long totalNodes;
void Driver(Board &board, int depth) {
  if (depth == 0) {
    totalNodes++;
    return;
  }
  Move::MoveList moveList;
  Move::generate(moveList, board);
  Board clone;
  for (int i = 0; i < moveList.count; i++) {
    // Clone the current state of the board
    clone = board;
    // Make move if it's not illegal (or check)
    if (!Move::make(&board, moveList.list[i], Move::allMoves))
      continue;

    Driver(board, depth - 1);

    // Restore board state
    board = clone;
  }
}

void Test(Board &board, int depth) {
  printf("\n----------------- Performance Test (%d) -----------------\n",
         depth);
  totalNodes = 0L;
  Move::MoveList moveList;
  Move::generate(moveList, board);
  Time::start();
  Board clone;
  for (int i = 0; i < moveList.count; i++) {
    // Clone the current state of the board
    clone = board;
    // Make move if it's not illegal (or check)
    if (!Move::make(&board, moveList.list[i], Move::allMoves))
      continue;

    long nodesSearchedSoFar = totalNodes;
    Driver(board, depth - 1);

    // Restore board state
    board = clone;

    printf("     %s: %ld\n", Move::toString(moveList.list[i]).c_str(),
           totalNodes - nodesSearchedSoFar);
  }
  printf("\n     Depth: %d\n", depth);
  printf("     Nodes: %d\n", totalNodes);
  printf("      Time: %lld ms\n", Time::end());
}
} // namespace Perft
