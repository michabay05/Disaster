#include "zobrist.hpp"

#include "bitboard.hpp"
#include "misc.hpp"

namespace Zobrist {
std::array<std::array<U64, 64>, 12> pieceKeys;
std::array<U64, 64> enpassKeys;
std::array<U64, 16> castlingKeys;
U64 sideKey;

std::array<std::array<U64, 64>, 12> pieceLocks;
std::array<U64, 64> enpassLocks;
std::array<U64, 16> castlingLocks;
U64 sideLock;

void init() {
  srand(0);
  // Init piece keys and locks
  for (int piece = P; piece <= k; piece++) {
    for (int sq = 0; sq <= 63; sq++) {
      pieceKeys[piece][sq] = random64();
      pieceLocks[piece][sq] = random64();
    }
  }

  // Init enpassant keys and locks
  for (int sq = 0; sq <= 63; sq++) {
    enpassKeys[sq] = random64();
    enpassLocks[sq] = random64();
  }

  // Init keys for the different castling rights variations
  for (int i = 0; i < 16; i++) {
    castlingKeys[i] = random64();
    castlingLocks[i] = random64();
  }

  sideKey = random64();
  sideLock = random64();
}

U64 genKey(const Board &board) {
  // Reset lock before generating
  U64 output = 0ULL;
  // Hash pieces on squares
  int sq;
  U64 bitboardCopy;
  for (int piece = P; piece <= k; piece++) {
    bitboardCopy = board.pieces[piece];
    while (bitboardCopy) {
      sq = Bitboard::getLs1bIndex(bitboardCopy);
      output ^= pieceKeys[piece][sq];
      popBit(bitboardCopy, sq);
    }
  }
  // Hash enpassant square
  if (board.enpassant != noSq)
    output ^= enpassKeys[board.enpassant];
  // Hash castling rights
  output ^= castlingKeys[board.castling];
  // Hash side to move
  if (board.side == BLACK)
    output ^= sideKey;
  return output;
}

U64 genLock(const Board &board) {
  // Reset lock before generating
  U64 output = 0ULL;
  // Hash pieces on squares
  int sq;
  U64 bitboardCopy;
  for (int piece = P; piece <= k; piece++) {
    bitboardCopy = board.pieces[piece];
    while (bitboardCopy) {
      sq = Bitboard::getLs1bIndex(bitboardCopy);
      output ^= pieceLocks[piece][sq];
      popBit(bitboardCopy, sq);
    }
  }
  // Hash enpassant square
  if (board.enpassant != noSq)
    output ^= enpassLocks[board.enpassant];
  // Hash castling rights
  output ^= castlingLocks[board.castling];
  // Hash side to move
  if (board.side == BLACK)
    output ^= sideLock;
  return output;
}
} // namespace Zobrist
