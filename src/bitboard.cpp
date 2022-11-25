#include "bitboard.hpp"

#include <cstdio>

namespace Bitboard {

void printBits(const U64 bitboard) {
  printf("\n");
  for (int r = 0; r < 8; r++) {
    printf("  %d |", 8 - r);
    for (int f = 0; f < 8; f++) {
      printf(" %d", getBit(bitboard, r * 8 + f));
    }
    printf("\n");
  }
  printf("      - - - - - - - -\n      a b c d e f g h\n");
  printf("\n\n      Decimal: %llu\n      Hexadecimal: 0x%llx\n", bitboard,
         bitboard);
}

}  // namespace Bitboard