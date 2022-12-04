#include "zobrist.hpp"

#include "bitboard.hpp"
#include "misc.hpp"

namespace Zobrist {
std::array<std::array<uint64_t, 64>, 12> pieceKeys;
std::array<uint64_t, 8> enpassKeys;
std::array<uint64_t, 16> castlingKeys;
uint64_t sideKey;

std::array<std::array<uint64_t, 64>, 12> pieceLocks;
std::array<uint64_t, 8> enpassLocks;
std::array<uint64_t, 16> castlingLocks;
uint64_t sideLock;

void init() {
  // Init piece keys and locks
  for (int piece = (int)Piece::P; piece <= (int)Piece::k; piece++) {
    for (int sq = 0; sq <= 63; sq++) {
      pieceKeys[piece][sq] = random64();
      pieceLocks[piece][sq] = random64();
    }
  }

  // Init enpassant files keys and locks
  for (int f = 0; f < 8; f++) {
    enpassKeys[f] = random64();
    enpassLocks[f] = random64();
  }

  // Init keys for the different castling rights variations
  for (int i = 0; i < 16; i++) {
    castlingKeys[i] = random64();
    castlingLocks[i] = random64();
  }

  sideKey = random64();
  sideLock = random64();
}

uint64_t genKey(const Board &board) {
  // Reset lock before generating
  uint64_t output = 0ULL;
  // Hash pieces on squares
  int sq;
  uint64_t bitboardCopy;
  for (int piece = (int)Piece::P; piece <= (int)Piece::k; piece++) {
    bitboardCopy = board.pos.pieces[piece];
    while (bitboardCopy) {
      sq = Bitboard::lsbIndex(bitboardCopy);
      output ^= pieceKeys[piece][sq];
      popBit(bitboardCopy, sq);
    }
  }
  // Hash enpassant square
  if (board.state.enpassant != Sq::noSq)
    output ^= enpassKeys[COL(board.state.enpassant)];
  // Hash castling rights
  output ^= castlingKeys[board.state.castling];
  // Hash side to move
  if (board.state.side == Color::BLACK)
    output ^= sideKey;
  return output;
}

uint64_t genLock(const Board &board) {
  // Reset lock before generating
  uint64_t output = 0ULL;
  // Hash pieces on squares
  int sq;
  uint64_t bitboardCopy;
  for (int piece = (int)Piece::P; piece <= (int)Piece::k; piece++) {
    bitboardCopy = board.pos.pieces[piece];
    while (bitboardCopy) {
      sq = Bitboard::lsbIndex(bitboardCopy);
      output ^= pieceLocks[piece][sq];
      popBit(bitboardCopy, sq);
    }
  }
  // Hash enpassant square
  if (board.state.enpassant != Sq::noSq)
    output ^= enpassLocks[COL(board.state.enpassant)];
  // Hash castling rights
  output ^= castlingLocks[board.state.castling];
  // Hash side to move
  if (board.state.side == Color::BLACK)
    output ^= sideLock;
  return output;
}

} // namespace Zobrist
