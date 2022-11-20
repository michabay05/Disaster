#include "eval.h"

#include "bitboard.h"

namespace Evaluate {
int materialScore[12] = {100,  300,  300,  500,  900,  10000,
                         -100, -300, -300, -500, -900, -10'000};

int EvalPosition(Board &board) {
  int score = 0;
  U64 bitboard;
  int sq;
  for (int piece = P; piece <= k; piece++) {
    bitboard = board.pieces[piece];
    while (bitboard) {
      sq = Bitboard::getLs1bIndex(bitboard);
      score += materialScore[piece];
      popBit(bitboard, sq);
    }
  }
  return (!board.side) ? score : -score;
}

} // namespace Evaluate