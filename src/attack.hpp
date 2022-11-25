#pragma once

#include "defs.hpp"

namespace Attack {
extern U64 pawnAttacks[2][64];       // [color][square]
extern U64 knightAttacks[64];       // [square]
extern U64 kingAttacks[64];         // [square]
extern U64 bishopOccMasks[64];       // [square]
extern U64 bishopAttacks[64][512];  // [square][occupancy variation]
extern U64 rookOccMasks[64];         // [square]
extern U64 rookAttacks[64][4096];   // [square][occupancy variation]
extern const int bishopRelevantBits[64]; // [square]
extern const int rookRelevantBits[64];   // [square]

// Prototypes
void init();
void initLeapers();
void initSliding(const int piece);
void genPawnAttacks(const int side, const int sq);
void genKnightAttacks(const int sq);
void genKingAttacks(const int sq);
U64 genBishopOccupancy(const int sq);
U64 genBishopAttack(const int sq, const U64 blockerBoard);
U64 genRookOccupancy(const int sq);
U64 genRookAttack(const int sq, const U64 blockerBoard);
U64 setOccupancy(const int index, const int relevantBits, U64 attackMask);

}
