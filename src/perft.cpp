#include "perft.hpp"

#include "bitboard.hpp"
#include "misc.hpp"
#include "move.hpp"
#include "zobrist.hpp"

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
    /* ============= FOR DEBUG PURPOSES ONLY ===============*/
    U64 updatedKey = board.hashKey;
    U64 updatedLock = board.hashLock;
    Zobrist::genKey(board);
    Zobrist::genLock(board);
    if (board.hashKey != updatedKey) {
      printf("\nBoard.MakeMove(%s)\n", Move::toString(moveList.list[i]).c_str());
      board.display();
      printf("Key should've been 0x%llx instead of 0x%llx\n", board.hashKey, updatedKey);
    }
    if (board.hashLock != updatedLock) {
      printf("\nBoard.MakeMove(%s)\n", Move::toString(moveList.list[i]).c_str());
      board.display();
      printf("Lock should've been 0x%llx instead of 0x%llx\n", board.hashLock, updatedLock);
    }
    /*============= FOR DEBUG PURPOSES ONLY =============== */
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
