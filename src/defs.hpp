#pragma once

#include <array>

// clang-format off
enum Squares {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1, noSq = -1
};
// clang-format on

#define U64 unsigned long long

#define ROW(sq) (sq >> 3)
#define COL(sq) (sq & 7)
#define SQ(r, f) (r * 8 + f)
#define FLIP(sq) (((7 - ROW(sq)) * 8) + COL(sq))

#define setBit(bitboard, square) ((bitboard) |= (1ULL << (square)))
#define getBit(bitboard, square) (((bitboard) & (1ULL << (square))) ? 1 : 0)
#define popBit(bitboard, square) ((bitboard) &= ~(1ULL << (square)))

/* Pieces */
enum Piece { P, N, B, R, Q, K, p, n, b, r, q, k, E };
enum PieceTypes { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };
enum Color { WHITE, BLACK, BOTH };
/* Direction offsets */
enum Direction {
  NORTH = 8,
  SOUTH = -8,
  WEST = -1,
  EAST = 1,
  NE = 9,     // NORTH + EAST
  NW = 7,     // NORTH + WEST
  SE = -7,    // SOUTH + EAST
  SW = -9,    // SOUTH + WEST
  NE_N = 17,  // 2(NORTH) + EAST -> 'KNIGHT ONLY'
  NE_E = 10,  // NORTH + 2(EAST) -> 'KNIGHT ONLY'
  NW_N = 15,  // 2(NORTH) + WEST -> 'KNIGHT ONLY'
  NW_W = 6,   // NORTH + 2(WEST) -> 'KNIGHT ONLY'
  SE_S = -15, // 2(SOUTH) + EAST -> 'KNIGHT ONLY'
  SE_E = -6,  // SOUTH + 2(EAST) -> 'KNIGHT ONLY'
  SW_S = -17, // 2(SOUTH) + WEST -> 'KNIGHT ONLY'
  SW_W = -10, // SOUTH + 2(WEST) -> 'KNIGHT ONLY'
};
