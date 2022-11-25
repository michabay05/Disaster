#include "attack.hpp"

#include <cassert>
#include <cstdio>

#include "bitboard.hpp"
#include "magics.hpp"

namespace Attack {

U64 pawnAttacks[2][64];       // [color][square]
U64 knightAttacks[64];       // [square]
U64 kingAttacks[64];         // [square]
U64 bishopOccMasks[64];       // [square]
U64 bishopAttacks[64][512];  // [square][occupancy variation]
U64 rookOccMasks[64];         // [square]
U64 rookAttacks[64][4096];   // [square][occupancy variation]

// clang-format off
const int bishopRelevantBits[64] = {
	6, 5, 5, 5, 5, 5, 5, 6,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 9, 9, 7, 5, 5,
	5, 5, 7, 7, 7, 7, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 5, 5, 5, 5, 5, 5, 6,
};

const int rookRelevantBits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	11, 10, 10, 10, 10, 10, 10, 11,
	12, 11, 11, 11, 11, 11, 11, 12,
};
// clang-format on

void init() {
  initLeapers();
  initSliding(BISHOP);
  initSliding(ROOK);
}

void initLeapers() {
  for (int sq = 0; sq < 64; sq++) {
    genPawnAttacks(WHITE, sq);
    genPawnAttacks(BLACK, sq);
    genKnightAttacks(sq);
    genKingAttacks(sq);
  }
}

void initSliding(const int piece) {
  for (int sq = 0; sq < 64; sq++) {
    bishopOccMasks[sq] = genBishopOccupancy(sq);
    rookOccMasks[sq] = genRookOccupancy(sq);

    U64 currentMask = (piece == BISHOP) ? bishopOccMasks[sq] : rookOccMasks[sq];
    int bitCount = Bitboard::countBits(currentMask);
    for (int count = 0; count < (1 << bitCount); count++) {
      U64 occupancy = setOccupancy(count, bitCount, currentMask);
      int magicInd;
      if (piece == BISHOP) {
        magicInd =
            (int)((occupancy * Magics::bishopMagics[sq]) >> (64 - bitCount));
        bishopAttacks[sq][magicInd] = genBishopAttack(sq, occupancy);
      }
      else {
        magicInd =
            (int)((occupancy * Magics::rookMagics[sq]) >> (64 - bitCount));
        rookAttacks[sq][magicInd] = genRookAttack(sq, occupancy);
      }
    }
  }
}

void genPawnAttacks(const int side, const int sq) {
  if (side == WHITE) {
    if (ROW(sq) > 0 && COL(sq) > 0) setBit(pawnAttacks[WHITE][sq], sq + SW);
    if (ROW(sq) > 0 && COL(sq) < 7) setBit(pawnAttacks[WHITE][sq], sq + SE);
  } else {
    if (ROW(sq) < 7 && COL(sq) > 0) setBit(pawnAttacks[BLACK][sq], sq + NW);
    if (ROW(sq) < 7 && COL(sq) < 7) setBit(pawnAttacks[BLACK][sq], sq + NE);
  }
}

void genKnightAttacks(const int sq) {
  if (ROW(sq) <= 5 && COL(sq) >= 1)
    setBit(knightAttacks[sq], sq + NW_N);

  if (ROW(sq) <= 6 && COL(sq) >= 2)
    setBit(knightAttacks[sq], sq + NW_W);

  if (ROW(sq) <= 6 && COL(sq) <= 5)
    setBit(knightAttacks[sq], sq + NE_E);

  if (ROW(sq) <= 5 && COL(sq) <= 6)
    setBit(knightAttacks[sq], sq + NE_N);

  if (ROW(sq) >= 2 && COL(sq) <= 6)
    setBit(knightAttacks[sq], sq + SE_S);

  if (ROW(sq) >= 1 && COL(sq) <= 5)
    setBit(knightAttacks[sq], sq + SE_E);

  if (ROW(sq) >= 1 && COL(sq) >= 2)
    setBit(knightAttacks[sq], sq + SW_W);

  if (ROW(sq) >= 2 && COL(sq) >= 1)
    setBit(knightAttacks[sq], sq + SW_S);
}

void genKingAttacks(const int sq) {
  if (ROW(sq) > 0) setBit(kingAttacks[sq], sq + SOUTH);
  if (ROW(sq) < 7) setBit(kingAttacks[sq], sq + NORTH);
  if (COL(sq) > 0) setBit(kingAttacks[sq], sq + WEST);
  if (COL(sq) < 7) setBit(kingAttacks[sq], sq + EAST);
  if (ROW(sq) > 0 && COL(sq) > 0) setBit(kingAttacks[sq], sq + SW);
  if (ROW(sq) > 0 && COL(sq) < 7) setBit(kingAttacks[sq], sq + SE);
  if (ROW(sq) < 7 && COL(sq) > 0) setBit(kingAttacks[sq], sq + NW);
  if (ROW(sq) < 7 && COL(sq) < 7) setBit(kingAttacks[sq], sq + NE);
}

U64 genBishopOccupancy(const int sq) {
  U64 output = 0ULL;
  int r, f;
  int sr = ROW(sq), sf = COL(sq);

  // NE direction
  for (r = sr + 1, f = sf + 1; r < 7 && f < 7; r++, f++)
    setBit(output, SQ(r, f));
  // NW direction
  for (r = sr + 1, f = sf - 1; r < 7 && f > 0; r++, f--)
    setBit(output, SQ(r, f));
  // SE direction
  for (r = sr - 1, f = sf + 1; r > 0 && f < 7; r--, f++)
    setBit(output, SQ(r, f));
  // SW direction
  for (r = sr - 1, f = sf - 1; r > 0 && f > 0; r--, f--)
    setBit(output, SQ(r, f));

  return output;
}

U64 genBishopAttack(const int sq, U64 blockerBoard) {
  U64 output = 0ULL;
  int r, f;
  int sr = ROW(sq), sf = COL(sq);

  // NE direction
  for (r = sr + 1, f = sf + 1; r <= 7 && f <= 7; r++, f++) {
    setBit(output, SQ(r, f));
    if (getBit(blockerBoard, SQ(r, f))) break;
  }
  // NW direction
  for (r = sr + 1, f = sf - 1; r <= 7 && f >= 0; r++, f--) {
    setBit(output, SQ(r, f));
    if (getBit(blockerBoard, SQ(r, f))) break;
  }
  // SE direction
  for (r = sr - 1, f = sf + 1; r >= 0 && f <= 7; r--, f++) {
    setBit(output, SQ(r, f));
    if (getBit(blockerBoard, SQ(r, f))) break;
  }
  // SW direction
  for (r = sr - 1, f = sf - 1; r >= 0 && f >= 0; r--, f--) {
    setBit(output, SQ(r, f));
    if (getBit(blockerBoard, SQ(r, f))) break;
  }

  return output;
}

U64 genRookOccupancy(const int sq) {
  U64 output = 0ULL;
  int r, f;
  int sr = ROW(sq), sf = COL(sq);

  // N direction
  for (r = sr + 1; r < 7; r++) setBit(output, SQ(r, sf));
  // S direction
  for (r = sr - 1; r > 0; r--) setBit(output, SQ(r, sf));
  // E direction
  for (f = sf + 1; f < 7; f++) setBit(output, SQ(sr, f));
  // W direction
  for (f = sf - 1; f > 0; f--) setBit(output, SQ(sr, f));

  return output;
}

U64 genRookAttack(const int sq, const U64 blockerBoard) {
  U64 output = 0ULL;
  int r, f;
  int sr = ROW(sq), sf = COL(sq);

  // N direction
  for (r = sr + 1; r <= 7; r++) {
    setBit(output, SQ(r, sf));
    if (getBit(blockerBoard, SQ(r, sf))) break;
  }
  // S direction
  for (r = sr - 1; r >= 0; r--) {
    setBit(output, SQ(r, sf));
    if (getBit(blockerBoard, SQ(r, sf))) break;
  }
  // E direction
  for (f = sf + 1; f <= 7; f++) {
    setBit(output, SQ(sr, f));
    if (getBit(blockerBoard, SQ(sr, f))) break;
  }
  // W direction
  for (f = sf - 1; f >= 0; f--) {
    setBit(output, SQ(sr, f));
    if (getBit(blockerBoard, SQ(sr, f))) break;
  }

  return output;
}

U64 setOccupancy(const int index, const int relevantBits, U64 attackMask) {
  U64 occupancy = 0ULL;
  for (int count = 0; count < relevantBits; count++) {
    int ls1bIndex = Bitboard::getLs1bIndex(attackMask);
    popBit(attackMask, ls1bIndex);
    if ((index & (1 << count)) > 0) setBit(occupancy, ls1bIndex);
  }
  return occupancy;
}
}  // namespace Attack