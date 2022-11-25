#pragma once

#include "board.hpp"
#include "defs.hpp"

namespace Zobrist {
extern std::array<std::array<U64, 64>, 12> pieceKeys;
extern std::array<U64, 64> enpassKeys;
extern std::array<U64, 16> castlingKeys;
extern U64 sideKey;

extern std::array<std::array<U64, 64>, 12> pieceLocks;
extern std::array<U64, 64> enpassLocks;
extern std::array<U64, 16> castlingLocks;
extern U64 sideLock;

void init();
U64 genKey(const Board &board);
U64 genLock(const Board &board);

inline void togglePiece(Board &board, const int piece, const int sq) {
  board.hashKey ^= pieceKeys[piece][sq];
  board.hashLock ^= pieceLocks[piece][sq];
}

inline void toggleEnpass(Board &board, const int sq) {
  board.hashKey ^= enpassKeys[sq];
  board.hashLock ^= enpassLocks[sq];
}

inline void toggleCastling(Board &board, const int castlingRights) {
  board.hashKey ^= castlingKeys[castlingRights];
  board.hashLock ^= castlingLocks[castlingRights];
}

inline void toggleSide(Board &board) {
  board.hashKey ^= sideKey;
  board.hashLock ^= sideLock;
}
} // namespace Zobrist
