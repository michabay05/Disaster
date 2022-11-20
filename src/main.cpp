#include <cstdio>

#include "attack.h"
#include "bitboard.h"
#include "board.h"
#include "eval.h"
#include "magics.h"
#include "move.h"
#include "perft.h"

#ifdef _DEBUG
#define DEBUG 1
#else
#define DEBUG 0
#endif

void test() {
    Board b;
    parseFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RN1QKBNR w KQkq - 0 1", b);
    b.display();
    printf("Evaluation: %d\n", Evaluate::EvalPosition(b));
}

int main(int argc, char **argv) {
  Attack::init();

  if (DEBUG)
    test();

  return 0;
}