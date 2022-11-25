#pragma once

#include "defs.hpp"

namespace Bitboard {

void printBits(const U64 bitboard);

inline int countBits(U64 bitboard) {
  int count = 0;
  for (count = 0; bitboard; count++, bitboard &= bitboard - 1)
    ;
  return count;
}
inline int getLs1bIndex(const U64 bitboard) {
  return bitboard > 0 ? countBits(bitboard ^ (bitboard - 1)) - 1 : 0;
}

} // namespace Bitboard