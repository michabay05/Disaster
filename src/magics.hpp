#pragma once

#include "defs.hpp"

namespace Magics {

extern const std::array<U64, 64> bishopMagics;
extern const std::array<U64, 64> rookMagics;

// Prototypes
U64 genRandomMagic();
U64 findMagicNumber(int sq, int relevantBits, int piece);
void initMagics();
U64 getBishopAttack(int sq, U64 blockerBoard);
U64 getRookAttack(int sq, U64 blockerBoard);
U64 getQueenAttack(int sq, U64 blockerBoard);

} // namespace Magics
