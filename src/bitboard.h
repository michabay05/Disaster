#pragma once

#include "defs.h"

namespace Bitboard {

void printBits(U64 bitboard);
int countBits(U64 bitboard);
int getLs1bIndex(U64 bitboard);
}