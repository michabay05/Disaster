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
#include "tt_eval.hpp"
#include "uci.hpp"
#include "zobrist.hpp"

#ifdef _DEBUG
#define DEBUG 1
#else
#define DEBUG 0
#endif

void test() {
    Board b;
    parseFen(position[1], b);
    b.display();
    Perft::test(b, 5);
}

int main(int argc, char** argv) {
    // Initializations
    Attack::init();
    Zobrist::init();
    TT::clearTTtable();
    TT::Eval::clearEvalTable();

    std::ios::sync_with_stdio(false);

    if (DEBUG)
        test();
    else
        UCI::loop();

    return 0;
}