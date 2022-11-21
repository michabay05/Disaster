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
void initSliding(int piece);
void genPawnAttacks(int side, int sq);
void genKnightAttacks(int sq);
void genKingAttacks(int sq);
U64 genBishopOccupancy(int sq);
U64 genBishopAttack(int sq, U64 blockerBoard);
U64 genRookOccupancy(int sq);
U64 genRookAttack(int sq, U64 blockerBoard);
U64 setOccupancy(int index, int relevantBits, U64 attackMask);

}
