#pragma once

#include "defs.hpp"

namespace Attack {
extern std::array<std::array<uint64_t, 64>, 2> pawnAttacks;     // [color][square]
extern std::array<uint64_t, 64> knightAttacks;                  // [square]
extern std::array<uint64_t, 64> kingAttacks;                    // [square]
extern std::array<uint64_t, 64> bishopOccMasks;                 // [square]
extern std::array<std::array<uint64_t, 512>, 64> bishopAttacks; // [square][occupancy variations]
extern std::array<uint64_t, 64> rookOccMasks;                   // [square]
extern std::array<std::array<uint64_t, 4096>, 64> rookAttacks;   // [square][occupancy variations]
extern const std::array<int, 64> bishopRelevantBits;            // [square]
extern const std::array<int, 64> rookRelevantBits;              // [square]

// Prototypes
void init();
void initLeapers();
void initSliding(const PieceTypes piece);
void genPawnAttacks(const Color side, const int sq);
void genKnightAttacks(const int sq);
void genKingAttacks(const int sq);
uint64_t genBishopOccupancy(const int sq);
uint64_t genBishopAttack(const int sq, const uint64_t blockerBoard);
uint64_t genRookOccupancy(const int sq);
uint64_t genRookAttack(const int sq, const uint64_t blockerBoard);
uint64_t setOccupancy(const int index, const int relevantBits, uint64_t attackMask);

} // namespace Attack
