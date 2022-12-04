#pragma once

#include "defs.hpp"

namespace Bitboard {

void printBits(const uint64_t bitboard);

inline int countBits(uint64_t bitboard) {
    int count = 0;
    for (count = 0; bitboard; count++, bitboard &= bitboard - 1)
        ;
    return count;
}
inline int lsbIndex(const uint64_t bitboard) {
    return bitboard > 0 ? countBits(bitboard ^ (bitboard - 1)) - 1 : 0;
}

} // namespace Bitboard