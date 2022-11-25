#include "eval.hpp"

#include "bitboard.hpp"

namespace Evaluate {
const int materialScore[12] = {
    100,    // white pawn score
    300,    // white knight scrore
    350,    // white bishop score
    500,    // white rook score
    1000,   // white queen score
    10000,  // white king score
    -100,   // black pawn score
    -300,   // black knight scrore
    -350,   // black bishop score
    -500,   // black rook score
    -1000,  // black queen score
    -10000, // black king score
};

// pawn positional score
const int pawnScore[64] = {
    90, 90, 90, 90,  90,  90, 90, 90, 30, 30, 30, 40, 40, 30, 30, 30,
    20, 20, 20, 30,  30,  30, 20, 20, 10, 10, 10, 20, 20, 10, 10, 10,
    5,  5,  10, 20,  20,  5,  5,  5,  0,  0,  0,  5,  5,  0,  0,  0,
    0,  0,  0,  -10, -10, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};

// knight positional score
const int knightScore[64] = {
    -5, 0,  0,  0,  0,  0,  0,  -5, -5, 0,   0,  10, 10, 0,  0,   -5,
    -5, 5,  20, 20, 20, 20, 5,  -5, -5, 10,  20, 30, 30, 20, 10,  -5,
    -5, 10, 20, 30, 30, 20, 10, -5, -5, 5,   20, 10, 10, 20, 5,   -5,
    -5, 0,  0,  0,  0,  0,  0,  -5, -5, -10, 0,  0,  0,  0,  -10, -5};

// bishop positional score
const int bishopScore[64] = {0,  0,  0,  0,  0, 0, 0,   0,  0,  0,   0,  0,  0,
                              0,  0,  0,  0,  0, 0, 10,  10, 0,  0,   0,  0,  0,
                              10, 20, 20, 10, 0, 0, 0,   0,  10, 20,  20, 10, 0,
                              0,  0,  10, 0,  0, 0, 0,   10, 0,  0,   30, 0,  0,
                              0,  0,  30, 0,  0, 0, -10, 0,  0,  -10, 0,  0

};

// rook positional score
const int rookScore[64] = {50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
                            50, 50, 50, 0,  0,  10, 20, 20, 10, 0,  0,  0,  0,
                            10, 20, 20, 10, 0,  0,  0,  0,  10, 20, 20, 10, 0,
                            0,  0,  0,  10, 20, 20, 10, 0,  0,  0,  0,  10, 20,
                            20, 10, 0,  0,  0,  0,  0,  20, 20, 0,  0,  0

};

// king positional score
const int kingScore[64] = {0,  0,  0,  0,  0,  0,  0,  0,  0,   0,  5,  5,  5,
                            5,  0,  0,  0,  5,  5,  10, 10, 5,   5,  0,  0,  5,
                            10, 20, 20, 10, 5,  0,  0,  5,  10,  20, 20, 10, 5,
                            0,  0,  0,  5,  10, 10, 5,  0,  0,   0,  5,  5,  -5,
                            -5, 0,  5,  0,  0,  0,  5,  0,  -15, 0,  10, 0};
int EvalPosition(const Board &board) {
  int score = 0;
  U64 bitboard;
  int sq;
  for (int piece = P; piece <= k; piece++) {
    bitboard = board.pieces[piece];
    while (bitboard) {
      sq = Bitboard::getLs1bIndex(bitboard);
      // Material score
      score += materialScore[piece];
      // score positional piece scores
      switch (piece) {
        // evaluate white pieces
      case P:
        score += pawnScore[sq];
        break;
      case N:
        score += knightScore[sq];
        break;
      case B:
        score += bishopScore[sq];
        break;
      case R:
        score += rookScore[sq];
        break;
      case K:
        score += kingScore[sq];
        break;

        // evaluate black pieces
      case p:
        score -= pawnScore[FLIP(sq)];
        break;
      case n:
        score -= knightScore[FLIP(sq)];
        break;
      case b:
        score -= bishopScore[FLIP(sq)];
        break;
      case r:
        score -= rookScore[FLIP(sq)];
        break;
      case k:
        score -= kingScore[FLIP(sq)];
        break;
      }
      popBit(bitboard, sq);
    }
  }
  return (!board.side) ? score : -score;
}

} // namespace Evaluate