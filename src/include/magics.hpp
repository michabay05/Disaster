#pragma once

#include "defs.hpp"

namespace Magics {

extern const std::array<uint64_t, 64> bishopMagics;
extern const std::array<uint64_t, 64> rookMagics;

// Prototypes
uint64_t findMagicNumber(const int sq, const int relevantBits, const PieceTypes piece);
void initMagics();
uint64_t getBishopAttack(const int sq, uint64_t blockerBoard);
uint64_t getRookAttack(const int sq, uint64_t blockerBoard);
uint64_t getQueenAttack(const int sq, uint64_t blockerBoard);

} // namespace Magics
