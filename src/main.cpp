#include <cstdio>

#include "attack.hpp"
#include "bitboard.hpp"
#include "board.hpp"
#include "eval.hpp"
#include "magics.hpp"
#include "misc.hpp"
#include "move.hpp"
#include "perft.hpp"
#include "search.hpp"
#include "tt.hpp"
#include "uci.hpp"
#include "zobrist.hpp"

#ifdef _DEBUG
#define DEBUG 1
#else
#define DEBUG 0
#endif

void test() {
  UCI::parse("position startpos moves e2e4 b8c6 b1c3 e7e5 g1f3 g8f6 d2d4 e5d4 "
             "f3d4 f8b4 f2f3 d7d5 d4c6 b7c6 e4d5 f6d5 c1d2 c8b7 d1e2 e8d7 c3d5 "
             "b4d2 e2d2 c6d5 e1c1 d7c8 g2g3 h8e8 f1g2 f7f5 f3f4 c7c6 h1e1 e8e1 "
             "d1e1 a7a5 a2a4 d8d7 h2h4 c8c7 e1e5 g7g6 d2e3 c7d8 e3c5 h7h5 c5f8 "
             "d8c7 f8f6 a8e8 f6g6 e8e5 f4e5 b7c8 g6f6 d7e6 c2c3 e6f6 e5f6 c7d6 "
             "b2b4 a5b4 c3b4 d6e6 g2f3 c8b7 f3h5 e6f6 h5f3 f6e5 f3g2 e5d4 c1b2 "
             "d4e3 g2h3 b7c8 a4a5 e3f3 h4h5 c8b7 h5h6 f5f4 h6h7 f3g3 h7h8q");
  UCI::parse("go depth 8");
}

int main(int argc, char **argv) {
  Attack::init();
  Zobrist::init();
  TT::init();

  if (DEBUG)
    test();
  else
    UCI::loop();

  return 0;
}