#include <cstdio>

#include "attack.hpp"
#include "bitboard.hpp"
#include "board.hpp"
#include "eval.hpp"
#include "magics.hpp"
#include "move.hpp"
#include "perft.hpp"
#include "search.hpp"
#include "uci.hpp"

#ifdef _DEBUG
#define DEBUG 1
#else
#define DEBUG 0
#endif

void test() {
    Board b;
    parseFen(position[2], b);
    Move::MoveList mL;
    Move::generate(mL, b);
    for (int i = 0; i < mL.count; i++)
        Search::scoreMoves(b, mL.list[i]);
}

int main(int argc, char **argv) {
  Attack::init();

  if (DEBUG)
    test();
  else
    UCI::loop();

  return 0;
}