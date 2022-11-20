#pragma once

#include "defs.h"

namespace Magics {

extern const U64 bishopMagics[64];
extern const U64 rookMagics[64];

// Prototypes
int random32();
U64 random64();
U64 genRandomMagic();
U64 findMagicNumber(int sq, int relevantBits, int piece);
void initMagics();
U64 getBishopAttack(int sq, U64 blockerBoard);
U64 getRookAttack(int sq, U64 blockerBoard);
U64 getQueenAttack(int sq, U64 blockerBoard);

}
